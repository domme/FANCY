#include "fancy_core_precompile.h"
#include "RenderOutputVk.h"
#include "Window.h"
#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"
#include "FancyCoreDefines.h"
#include "GpuResource.h"
#include "GpuResourceDataVk.h"
#include "TextureVk.h"
#include "CommandQueueVk.h"
#include "CommandListVk.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
//---------------------------------------------------------------------------//
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

      myBackbufferFormat = backbufferFormat;
      myBackbufferColorSpace = backbufferColorSpace;
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
      myPresentMode = bestPresentMode;
    }

    // Pick the appropriate sharing mode. 
    {
      const RenderPlatformCaps& caps = RenderCore::GetPlatformCaps();
      const bool hasAsyncQueues = caps.myHasAsyncCompute || caps.myHasAsyncCopy;

      // In single - queue scenarios it is a validation error to pick CONCURRENT
      mySharingMode = hasAsyncQueues ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    }

    VkSurfaceCapabilitiesKHR surfaceCaps;
    ASSERT_VK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(platformVk->myPhysicalDevice, mySurface, &surfaceCaps));
    myNumBackbuffers = glm::clamp(kBackbufferCount, surfaceCaps.minImageCount, surfaceCaps.maxImageCount);

    CreateSwapChain(myWindow->GetWidth(), myWindow->GetHeight());

    VkFenceCreateInfo fenceCreateInfo;
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = nullptr;
    fenceCreateInfo.flags = 0u;
    ASSERT_VK_RESULT(vkCreateFence(platformVk->myDevice, &fenceCreateInfo, nullptr, &myBackbufferReadyFence));
  }
  //---------------------------------------------------------------------------//
  RenderOutputVk::~RenderOutputVk()
  {
    DestroySwapChain();
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    vkDestroySurfaceKHR(platformVk->myInstance, mySurface, nullptr);
  }
//---------------------------------------------------------------------------//
  void RenderOutputVk::CreateSwapChain(uint aWidth, uint aHeight)
  {
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

    VkSurfaceCapabilitiesKHR surfaceCaps;
    ASSERT_VK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(platformVk->myPhysicalDevice, mySurface, &surfaceCaps));

    VkExtent2D swapChainRes = surfaceCaps.currentExtent;
    if (swapChainRes.width == UINT_MAX)
      swapChainRes.width = glm::clamp(aWidth, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
    if (swapChainRes.height == UINT_MAX)
      swapChainRes.height = glm::clamp(aHeight, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.surface = mySurface;
    createInfo.minImageCount = myNumBackbuffers;
    createInfo.imageExtent = swapChainRes;
    createInfo.imageColorSpace = myBackbufferColorSpace;
    createInfo.imageFormat = myBackbufferFormat;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    createInfo.imageSharingMode = mySharingMode;
    createInfo.preTransform = surfaceCaps.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = myPresentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = nullptr;  // Could be used for a more optimal swapchain-recreation
    ASSERT_VK_RESULT(vkCreateSwapchainKHR(platformVk->myDevice, &createInfo, nullptr, &mySwapChain));
  }
//---------------------------------------------------------------------------//
  void RenderOutputVk::DestroySwapChain()
  {
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    ASSERT_VK_RESULT(vkDeviceWaitIdle(platformVk->myDevice));
    vkDestroySwapchainKHR(platformVk->myDevice, mySwapChain, nullptr);
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

    DataFormat backbufferFormat = DataFormat::UNKNOWN;
    switch (myBackbufferFormat)
    {
    case VK_FORMAT_R8G8B8A8_UNORM:
      backbufferFormat = DataFormat::RGBA_8;
      break;
    case VK_FORMAT_B8G8R8A8_UNORM:
      backbufferFormat = DataFormat::BGRA_8;
      break;
    default:
      ASSERT(false, "Unsupported backbuffer format");
    }

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

      resource.mySubresources = SubresourceRange(0u, 1u, 0u, 1u, 0u, 1u);

      resource.myStateTracking = GpuResourceHazardData();
      resource.myStateTracking.myVkData.myReadAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;
      resource.myStateTracking.myVkData.myWriteAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
      resource.myStateTracking.myVkData.myHasExclusiveQueueAccess = mySharingMode == VK_SHARING_MODE_EXCLUSIVE;
      resource.myStateTracking.myVkData.mySupportedImageLayouts.push_back(VK_IMAGE_LAYOUT_GENERAL);
      resource.myStateTracking.myVkData.mySupportedImageLayouts.push_back(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
      resource.myStateTracking.myVkData.mySupportedImageLayouts.push_back(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
      resource.myStateTracking.myVkData.mySupportedImageLayouts.push_back(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      resource.myStateTracking.myVkData.mySupportedImageLayouts.push_back(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      resource.myStateTracking.myVkData.mySupportedImageLayouts.push_back(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
      resource.myStateTracking.myVkData.mySupportedImageLayouts.push_back(VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR);

      GpuSubresourceHazardDataVk subHazardData;
      subHazardData.myContext = CommandListType::Graphics;
      subHazardData.myAccessMask = 0u;
      subHazardData.myImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      resource.myStateTracking.myVkData.mySubresources.resize(1u, subHazardData);

      TextureProperties backbufferProps;
      backbufferProps.myDimension = GpuResourceDimension::TEXTURE_2D;
      backbufferProps.myIsRenderTarget = true;
      backbufferProps.myFormat = backbufferFormat;
      backbufferProps.myWidth = aWidth;
      backbufferProps.myHeight = aHeight;
      backbufferProps.myDepthOrArraySize = 1u;
      backbufferProps.myNumMipLevels = 1u;

      myBackbufferTextures[i].reset(new TextureVk(std::move(resource), backbufferProps, true));
      myBackbufferTextures[i]->SetName(name.GetBuffer());
    }

    myBackbuffersUsed.resize(myNumBackbuffers);
    std::fill_n(myBackbuffersUsed.begin(), myBackbuffersUsed.size(), false);
  }
  //---------------------------------------------------------------------------//
  void RenderOutputVk::ResizeSwapChain(uint aWidth, uint aHeight)
  {
    DestroySwapChain();
    CreateSwapChain(aWidth, aHeight);
  }
  //---------------------------------------------------------------------------//
  void RenderOutputVk::DestroyBackbufferResources()
  {
    for (uint i = 0u; i < kBackbufferCount; ++i)
    {
      myBackbufferTextures[i].reset();
    }
  }
  //---------------------------------------------------------------------------//
  void RenderOutputVk::OnBeginFrame()
  {
    VkDevice device = RenderCore::GetPlatformVk()->myDevice;
    ASSERT_VK_RESULT(vkAcquireNextImageKHR(device, mySwapChain, UINT64_MAX, nullptr, myBackbufferReadyFence, &myCurrBackbufferIndex));
    ASSERT_VK_RESULT(vkWaitForFences(device, 1u, &myBackbufferReadyFence, true, UINT64_MAX));
    ASSERT_VK_RESULT(vkResetFences(device, 1u, &myBackbufferReadyFence));

    // This should've only been necessary with the manual resource transitions
    /*

    // Upon first use, each backbuffer must be transitioned from an unknown image layout into the present-layout that high-level rendering code expects
    if (!myBackbuffersUsed[myCurrBackbufferIndex])
    {
      myBackbuffersUsed[myCurrBackbufferIndex] = true;

      CommandListVk* ctx = static_cast<CommandListVk*>(RenderCore::BeginCommandList(CommandListType::Graphics));
      const ResourceBarrierInfoVk presentInfo = RenderCore_PlatformVk::ResolveResourceState(GpuResourceState::READ_PRESENT);
      ctx->SubresourceBarrierInternal(myBackbufferTextures[myCurrBackbufferIndex].get(), myBackbufferTextures[myCurrBackbufferIndex]->GetSubresources(),
        0u,
        presentInfo.myAccessMask,
        0u,
        presentInfo.myStageMask,
        VK_IMAGE_LAYOUT_UNDEFINED,
        presentInfo.myImageLayout,
        CommandListType::Graphics,
        CommandListType::Graphics
      );
      RenderCore::ExecuteAndFreeCommandList(ctx);
    }
    */
  }
  //---------------------------------------------------------------------------//
  void RenderOutputVk::Present()
  {
    const CommandQueueVk* graphicsQueue = static_cast<CommandQueueVk*>(RenderCore::GetCommandQueue(CommandListType::Graphics));

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.swapchainCount = 1u;
    presentInfo.pResults = nullptr;
    presentInfo.pImageIndices = &myCurrBackbufferIndex;
    presentInfo.pSwapchains = &mySwapChain;
    presentInfo.pWaitSemaphores = nullptr;  // Wait-semaphores should be unnecessary when the present-queue is the graphics-queue.
    presentInfo.waitSemaphoreCount = 0u;
    ASSERT_VK_RESULT(vkQueuePresentKHR(graphicsQueue->GetQueue(), &presentInfo));
  }
//---------------------------------------------------------------------------//
}

#endif