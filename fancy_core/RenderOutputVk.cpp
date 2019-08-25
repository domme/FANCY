#include "fancy_core_precompile.h"
#include "RenderOutputVk.h"
#include "Window.h"
#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"
#include "FancyCoreDefines.h"
#include "GpuResource.h"
#include "GpuResourceDataVk.h"
#include "TextureVk.h"

namespace Fancy
{
  RenderOutputVk::RenderOutputVk(void* aNativeInstanceHandle, const WindowParameters& someWindowParams)
    : RenderOutput(aNativeInstanceHandle, someWindowParams)
  {
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

    // Create the surface
    {
      VkWin32SurfaceCreateInfoKHR createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
      createInfo.hwnd = myWindow->GetWindowHandle();
      createInfo.hinstance = reinterpret_cast<HINSTANCE>(aNativeInstanceHandle);
      ASSERT_VK_RESULT(vkCreateWin32SurfaceKHR(platformVk->myInstance, &createInfo, nullptr, &mySurface));
    }

    // Ensure that the graphics queue can be used as a present queue
    {
      VkBool32 supportsPresent = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(platformVk->myPhysicalDevice,
        platformVk->myQueueInfos[(uint)CommandListType::Graphics].myQueueFamilyIndex,
        mySurface, &supportsPresent);

      ASSERT(supportsPresent, "Vulkan graphics queue doesn't support present. There's currently no support for presenting on a different queue");
    }

    const VkFormat backbufferFormat = VK_FORMAT_B8G8R8A8_UNORM;
    const VkColorSpaceKHR backbufferColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    // Ensure that the implementation supports the desired backbuffer formats
    {
      uint numFormats = 0u;
      ASSERT_VK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(platformVk->myPhysicalDevice, mySurface, &numFormats, nullptr));

      VkSurfaceFormatKHR* formats = (VkSurfaceFormatKHR*)alloca(sizeof(VkSurfaceFormatKHR) * numFormats);
      ASSERT_VK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(platformVk->myPhysicalDevice, mySurface, &numFormats, formats));

      bool formatSupported = false;
      bool colorSpaceSupported = false;
      for (uint i = 0u; i < numFormats && !(formatSupported && colorSpaceSupported); ++i)
      {
        const VkSurfaceFormatKHR& format = formats[i];
        formatSupported |= format.format == backbufferFormat;
        colorSpaceSupported |= format.colorSpace == backbufferColorSpace;
      }
      ASSERT(formatSupported, "Required backbuffer format not supported for swapchain creation");
      ASSERT(colorSpaceSupported, "Required color space not supported for swapchain creation");
    }
    
    VkPresentModeKHR bestPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    // Choose a present mode
    {
      uint numPresentModes = 0u;
      ASSERT_VK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(platformVk->myPhysicalDevice, mySurface, &numPresentModes, nullptr));

      VkPresentModeKHR* presentModes = (VkPresentModeKHR*)alloca(sizeof(VkPresentModeKHR) * numPresentModes);
      ASSERT_VK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(platformVk->myPhysicalDevice, mySurface, &numPresentModes, presentModes));

      uint presentModePriorities[VK_PRESENT_MODE_RANGE_SIZE_KHR] = { 0u };
      presentModePriorities[VK_PRESENT_MODE_MAILBOX_KHR] = 4u;
      presentModePriorities[VK_PRESENT_MODE_FIFO_RELAXED_KHR] = 3u;
      presentModePriorities[VK_PRESENT_MODE_FIFO_KHR] = 2u;
      presentModePriorities[VK_PRESENT_MODE_IMMEDIATE_KHR] = 1u;

      for (uint i = 0u; i < numPresentModes; ++i)
      {
        VkPresentModeKHR presentMode = presentModes[i];
        const uint prio = presentModePriorities[presentMode];
        const uint oldPrio = presentModePriorities[bestPresentMode];

        if (prio > oldPrio)
          bestPresentMode = presentMode;
      }
      ASSERT(presentModePriorities[bestPresentMode] > 0u, "None of the desired present mode options seems to be supported for Vulkan swap chain creation");
    }

    VkExtent2D swapChainRes;
    VkSurfaceCapabilitiesKHR surfaceCaps;
    ASSERT_VK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(platformVk->myPhysicalDevice, mySurface, &surfaceCaps));

    swapChainRes = surfaceCaps.currentExtent;
    if (swapChainRes.width == UINT_MAX)
      swapChainRes.width = glm::clamp(myWindow->GetWidth(), surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
    if (swapChainRes.height == UINT_MAX)
      swapChainRes.height = glm::clamp(myWindow->GetHeight(), surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);

    myNumBackbuffers = glm::clamp(kBackbufferCount, surfaceCaps.minImageCount, surfaceCaps.maxImageCount);

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = mySurface;
    createInfo.minImageCount = myNumBackbuffers;
    createInfo.imageExtent = swapChainRes;
    createInfo.imageColorSpace = backbufferColorSpace;
    createInfo.imageFormat = backbufferFormat;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;  // SHARING would only be needed if the present-queue is different from the graphics-queue, which we don't support currently.
    createInfo.preTransform = surfaceCaps.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = bestPresentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = nullptr;  // Needed for swapchain-recreation after resizing later...

    ASSERT_VK_RESULT(vkCreateSwapchainKHR(platformVk->myDevice, &createInfo, nullptr, &mySwapChain));

    // TODO(Vk): This call is just for testing purposes:
    CreateBackbufferResources(swapChainRes.width, swapChainRes.height);
  }
//---------------------------------------------------------------------------//
  RenderOutputVk::~RenderOutputVk()
  {
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    vkDestroySwapchainKHR(platformVk->myDevice, mySwapChain, nullptr);
    vkDestroySurfaceKHR(platformVk->myInstance, mySurface, nullptr);
  }
//---------------------------------------------------------------------------//
  void RenderOutputVk::CreateBackbufferResources(uint aWidth, uint aHeight)
  {
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

    uint numSwapChainImages;
    vkGetSwapchainImagesKHR(platformVk->myDevice, mySwapChain, &numSwapChainImages, nullptr);
    ASSERT(myNumBackbuffers == numSwapChainImages);

    VkImage* const swapChainImages = (VkImage*)alloca(sizeof(VkImage*) * numSwapChainImages);
    vkGetSwapchainImagesKHR(platformVk->myDevice, mySwapChain, &numSwapChainImages, swapChainImages);

    for (uint i = 0u; i < myNumBackbuffers; ++i)
    {
      StaticString<32> name("Backbuffer Texture %d", i);

      GpuResource resource(GpuResourceCategory::TEXTURE);
      resource.myName = name.GetBuffer();

      {
        GpuResourceDataVk* dataVk(new GpuResourceDataVk);
        dataVk->myType = GpuResourceCategory::TEXTURE;
        dataVk->myImage = swapChainImages[i];
        resource.myNativeData = dataVk;
      }

      resource.myStateTracking = GpuResourceStateTracking();
      // TODO(Vk): Add similar read- and write-state masks as in DX12?
      resource.myStateTracking.myDefaultState = GpuResourceState::READ_PRESENT;

      TextureProperties backbufferProps;
      backbufferProps.myDimension = GpuResourceDimension::TEXTURE_2D;
      backbufferProps.myIsRenderTarget = true;
      backbufferProps.eFormat = DataFormat::RGBA_8;
      backbufferProps.myWidth = aWidth;
      backbufferProps.myHeight = aHeight;
      backbufferProps.myDepthOrArraySize = 1u;
      backbufferProps.myNumMipLevels = 1u;

      myBackbufferTextures[i].reset(new TextureVk(std::move(resource), backbufferProps));
      myBackbufferTextures[i]->SetName(name.GetBuffer());
    }
  }

  void RenderOutputVk::ResizeBackbuffer(uint aWidth, uint aHeight)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }

  void RenderOutputVk::DestroyBackbufferResources()
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }

  void RenderOutputVk::OnBeginFrame()
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }

  void RenderOutputVk::Present()
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
}
