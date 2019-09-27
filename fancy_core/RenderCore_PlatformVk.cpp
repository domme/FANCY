#include "fancy_core_precompile.h"

#include "RenderCore_PlatformVk.h"
#include "RendererPrerequisites.h"
#include "RenderOutputVk.h"
#include "CommandQueueVk.h"
#include "ShaderCompilerVk.h"
#include "ShaderVk.h"
#include "ShaderPipelineVk.h"
#include "TextureVk.h"
#include "CommandListVk.h"

namespace Fancy
{
  class CommandQueueVk;

  namespace
  {
    void locPrintAvailableInstanceExtensions()
    {
      uint numAvailableExtensions = 0u;
      vkEnumerateInstanceExtensionProperties(nullptr, &numAvailableExtensions, nullptr);

      DynamicArray<VkExtensionProperties> availableExtensionProperties;
      availableExtensionProperties.resize(numAvailableExtensions);
      vkEnumerateInstanceExtensionProperties(nullptr, &numAvailableExtensions, availableExtensionProperties.data());

      LOG("Available Vulkan instance extensions:");
      for (const VkExtensionProperties& prop : availableExtensionProperties)
        LOG("%s (spec version %d)", prop.extensionName, prop.specVersion);
    }

    void locPrintAvailableDeviceExtensions(VkPhysicalDevice aPhysicalDevice)
    {
      uint numAvailableExtensions = 0u;
      vkEnumerateDeviceExtensionProperties(aPhysicalDevice, nullptr, &numAvailableExtensions, nullptr);

      DynamicArray<VkExtensionProperties> availableExtensionProperties (numAvailableExtensions);
      vkEnumerateDeviceExtensionProperties(aPhysicalDevice, nullptr, &numAvailableExtensions, availableExtensionProperties.data());

      LOG("Available Vulkan device extensions:");
      for (const VkExtensionProperties& prop : availableExtensionProperties)
        LOG("%s (spec version %d)", prop.extensionName, prop.specVersion);
    }

    void locPrintAvailableLayers()
    {
      uint numAvailableLayers = 0u;
      vkEnumerateInstanceLayerProperties(&numAvailableLayers, nullptr);

      DynamicArray<VkLayerProperties> availableLayerProperties;
      availableLayerProperties.resize(numAvailableLayers);
      vkEnumerateInstanceLayerProperties(&numAvailableLayers, availableLayerProperties.data());

      LOG("Available Vulkan Layers:");
      for (const VkLayerProperties& prop : availableLayerProperties)
        LOG("%s (spec version %d, implementation version %d)", prop.layerName, prop.specVersion, prop.implementationVersion);
    }

    uint64 locEvaluateDevice(const VkPhysicalDeviceProperties& someProps, const VkPhysicalDeviceFeatures& someFeatures)
    {
      uint64 score = 0u;

      score += someProps.limits.maxImageDimension1D;
      score += someProps.limits.maxImageDimension2D;
      score += someProps.limits.maxImageDimension3D;
      score += someProps.limits.maxImageDimensionCube;
      score += someProps.limits.maxImageArrayLayers;
      score += someProps.limits.maxTexelBufferElements;
      score += someProps.limits.maxUniformBufferRange;
      score += someProps.limits.maxStorageBufferRange;
      score += someProps.limits.maxPushConstantsSize;
      score += someProps.limits.maxMemoryAllocationCount;
      score += someProps.limits.maxSamplerAllocationCount;
      score += someProps.limits.bufferImageGranularity;
      score += someProps.limits.sparseAddressSpaceSize;
      score += someProps.limits.maxBoundDescriptorSets;
      score += someProps.limits.maxPerStageDescriptorSamplers;
      score += someProps.limits.maxPerStageDescriptorUniformBuffers;
      score += someProps.limits.maxPerStageDescriptorStorageBuffers;
      score += someProps.limits.maxPerStageDescriptorSampledImages;
      score += someProps.limits.maxPerStageDescriptorStorageImages;
      score += someProps.limits.maxPerStageDescriptorInputAttachments;
      score += someProps.limits.maxPerStageResources;
      score += someProps.limits.maxDescriptorSetSamplers;
      score += someProps.limits.maxDescriptorSetUniformBuffers;
      score += someProps.limits.maxDescriptorSetUniformBuffersDynamic;
      score += someProps.limits.maxDescriptorSetStorageBuffers;
      score += someProps.limits.maxDescriptorSetStorageBuffersDynamic;
      score += someProps.limits.maxDescriptorSetSampledImages;
      score += someProps.limits.maxDescriptorSetStorageImages;
      score += someProps.limits.maxDescriptorSetInputAttachments;
      score += someProps.limits.maxVertexInputAttributes;
      score += someProps.limits.maxVertexInputBindings;
      score += someProps.limits.maxVertexInputAttributeOffset;
      score += someProps.limits.maxVertexInputBindingStride;
      score += someProps.limits.maxVertexOutputComponents;
      score += someProps.limits.maxTessellationGenerationLevel;
      score += someProps.limits.maxTessellationPatchSize;
      score += someProps.limits.maxTessellationControlPerVertexInputComponents;
      score += someProps.limits.maxTessellationControlPerVertexOutputComponents;
      score += someProps.limits.maxTessellationControlPerPatchOutputComponents;
      score += someProps.limits.maxTessellationControlTotalOutputComponents;
      score += someProps.limits.maxTessellationEvaluationInputComponents;
      score += someProps.limits.maxTessellationEvaluationOutputComponents;
      score += someProps.limits.maxGeometryShaderInvocations;
      score += someProps.limits.maxGeometryInputComponents;
      score += someProps.limits.maxGeometryOutputComponents;
      score += someProps.limits.maxGeometryOutputVertices;
      score += someProps.limits.maxGeometryTotalOutputComponents;
      score += someProps.limits.maxFragmentInputComponents;
      score += someProps.limits.maxFragmentOutputAttachments;
      score += someProps.limits.maxFragmentDualSrcAttachments;
      score += someProps.limits.maxFragmentCombinedOutputResources;
      score += someProps.limits.maxComputeSharedMemorySize;
      for (uint val : someProps.limits.maxComputeWorkGroupCount)
        score += val;
      score += someProps.limits.maxComputeWorkGroupInvocations;
      for (uint val : someProps.limits.maxComputeWorkGroupSize)
        score += val;
      score += someProps.limits.subPixelPrecisionBits;
      score += someProps.limits.subTexelPrecisionBits;
      score += someProps.limits.mipmapPrecisionBits;
      score += someProps.limits.maxDrawIndexedIndexValue;
      score += someProps.limits.maxDrawIndirectCount;
      score += someProps.limits.maxViewports;
      for (uint val : someProps.limits.maxViewportDimensions)
        score += val;

      return score;
    }
  }

  VkFormat RenderCore_PlatformVk::ResolveFormat(DataFormat aFormat)
  {
    switch (aFormat)
    {
    case SRGB_8_A_8:    return VK_FORMAT_R8G8B8A8_SRGB;
    case RGBA_8:        return VK_FORMAT_R8G8B8A8_UNORM;
    case RG_8:          return VK_FORMAT_R8G8_UNORM;
    case R_8:           return VK_FORMAT_R8_UNORM;
    case RGBA_16:       return VK_FORMAT_R16G16B16A16_UNORM;
    case RG_16:         return VK_FORMAT_R16G16_UNORM;
    case R_16:          return VK_FORMAT_R16_UNORM;
    case RGB_11_11_10F: return VK_FORMAT_B10G11R11_UFLOAT_PACK32; // Memory-layout in both DX and Vk has B in the 10 highest bits. TODO: Check if we need additional swizzling in Vulkan-shaders
    case RGBA_16F:      return VK_FORMAT_R16G16B16A16_SFLOAT;
    case RGB_16F:       return VK_FORMAT_R16G16B16_SFLOAT;
    case RG_16F:        return VK_FORMAT_R16G16_SFLOAT;
    case R_16F:         return VK_FORMAT_R16_SFLOAT;
    case RGBA_32F:      return VK_FORMAT_R32G32B32A32_SFLOAT;
    case RGB_32F:       return VK_FORMAT_R32G32B32_SFLOAT;
    case RG_32F:        return VK_FORMAT_R32G32_SFLOAT;
    case R_32F:         return VK_FORMAT_R32_SFLOAT;
    case RGBA_32UI:     return VK_FORMAT_R32G32B32A32_UINT;
    case RGB_32UI:      return VK_FORMAT_R32G32B32_UINT;
    case RG_32UI:       return VK_FORMAT_R32G32_UINT;
    case R_32UI:        return VK_FORMAT_R32_UINT;
    case RGBA_16UI:     return VK_FORMAT_R16G16B16A16_UINT;
    case RGB_16UI:      return VK_FORMAT_R16G16B16_UINT;
    case RG_16UI:       return VK_FORMAT_R16G16_UINT;
    case R_16UI:        return VK_FORMAT_R16_UINT;
    case RGBA_8UI:      return VK_FORMAT_R8G8B8A8_UINT;
    case RGB_8UI:       return VK_FORMAT_R8G8B8_UINT;
    case RG_8UI:        return VK_FORMAT_R8G8_UINT;
    case R_8UI:         return VK_FORMAT_R8_UINT;
    case RGBA_32I:      return VK_FORMAT_R32G32B32A32_SINT;
    case RGB_32I:       return VK_FORMAT_R32G32B32_SINT;
    case RG_32I:        return VK_FORMAT_R32G32_SINT;
    case R_32I:         return VK_FORMAT_R32_SINT;
    case RGBA_16I:      return VK_FORMAT_R16G16B16A16_SINT;
    case RGB_16I:       return VK_FORMAT_R16G16B16_SINT;
    case RG_16I:        return VK_FORMAT_R16G16_SINT;
    case R_16I:         return VK_FORMAT_R16_SINT;
    case RGBA_8I:       return VK_FORMAT_R8G8B8A8_SINT;
    case RGB_8I:        return VK_FORMAT_R8G8B8_SINT;
    case RG_8I:         return VK_FORMAT_R8G8_SINT;
    case R_8I:          return VK_FORMAT_R8_SINT;
    case D_24UNORM_S_8UI: return VK_FORMAT_D24_UNORM_S8_UINT;
    default: ASSERT(false, "Unsupported format"); return VK_FORMAT_R8G8B8A8_SRGB;
    }
  }
//---------------------------------------------------------------------------//
  VkBlendFactor RenderCore_PlatformVk::ResolveBlendFactor(BlendFactor aFactor)
  {
    static constexpr VkBlendFactor typeTable[] =
    {
      VK_BLEND_FACTOR_ZERO,
      VK_BLEND_FACTOR_ONE,
      VK_BLEND_FACTOR_SRC_COLOR,
      VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR, 
      VK_BLEND_FACTOR_SRC_ALPHA,
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
      VK_BLEND_FACTOR_DST_ALPHA,
      VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
      VK_BLEND_FACTOR_DST_COLOR,
      VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
      VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
      VK_BLEND_FACTOR_SRC1_COLOR,
      VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
      VK_BLEND_FACTOR_SRC1_ALPHA,
      VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA,
      VK_BLEND_FACTOR_CONSTANT_COLOR,
      VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR
    };

  }
//---------------------------------------------------------------------------//
  VkBlendOp RenderCore_PlatformVk::ResolveBlendOp(BlendOp aBlendOp)
  {
  }
//---------------------------------------------------------------------------//
  RenderCore_PlatformVk::RenderCore_PlatformVk() : RenderCore_Platform(RenderPlatformType::VULKAN)
  {
    LOG("Initializing Vulkan device...");
    locPrintAvailableInstanceExtensions();
    locPrintAvailableLayers();

    // Create instance
    {
      VkApplicationInfo appInfo = {};
      appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      appInfo.pApplicationName = "Fancy";
      appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
      appInfo.apiVersion = VK_API_VERSION_1_0;

      VkInstanceCreateInfo createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      createInfo.pApplicationInfo = &appInfo;

      const char* const extensions[] = { 
        VK_KHR_SURFACE_EXTENSION_NAME, 
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME, 
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
      };

      createInfo.enabledExtensionCount = ARRAY_LENGTH(extensions);
      createInfo.ppEnabledExtensionNames = extensions;

      const char* const layers[] = { "VK_LAYER_KHRONOS_validation" };
      createInfo.enabledLayerCount = ARRAY_LENGTH(layers);
      createInfo.ppEnabledLayerNames = layers;

      ASSERT_VK_RESULT(vkCreateInstance(&createInfo, nullptr, &myInstance));
      LOG("Initialized Vulkan instance");

      VkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT) vkGetInstanceProcAddr(myInstance, "vkSetDebugUtilsObjectNameEXT");
      ASSERT(VkSetDebugUtilsObjectNameEXT != nullptr);
    }

    // Create physical device
    {
      uint numDevices = 0u;
      vkEnumeratePhysicalDevices(myInstance, &numDevices, nullptr);
      ASSERT(numDevices > 0u, "No Vulkan-capable physical devices found!");

      DynamicArray<VkPhysicalDevice> devices(numDevices);
      vkEnumeratePhysicalDevices(myInstance, &numDevices, devices.data());

      LOG("Picking a suitable Vulkan device...");
      uint64 highestScore = 0ull;
      uint highestScoreIdx = 0u;
      for (uint i = 0u, e = (uint)devices.size(); i < e; ++i)
      {
        const VkPhysicalDevice& device = devices[i];

        VkPhysicalDeviceProperties deviceProps;
        vkGetPhysicalDeviceProperties(device, &deviceProps);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        const uint64 score = locEvaluateDevice(deviceProps, deviceFeatures);
        LOG("Device %d (%s) - score: %llu", i, deviceProps.deviceName, score);

        if (score > highestScore)
        {
          highestScoreIdx = i;
          highestScore = score;
        }
      }

      LOG("...Picked device %d as Vulkan render device", highestScoreIdx);
      myPhysicalDevice = devices[highestScoreIdx];
      vkGetPhysicalDeviceFeatures(myPhysicalDevice, &myPhysicalDeviceFeatures);
      vkGetPhysicalDeviceProperties(myPhysicalDevice, &myPhysicalDeviceProperties);
    }

    // Create queues and logical device
    {
      uint numQueueFamilies = 0u;
      vkGetPhysicalDeviceQueueFamilyProperties(myPhysicalDevice, &numQueueFamilies, nullptr);
      ASSERT(numQueueFamilies > 0u, "Invalid queue family count");

      DynamicArray<VkQueueFamilyProperties> queueFamilyProps(numQueueFamilies);
      vkGetPhysicalDeviceQueueFamilyProperties(myPhysicalDevice, &numQueueFamilies, queueFamilyProps.data());

      // Try to get the most suitable queues for graphics, compute and dma.
      // First try to use queues from different families (assuming different queue-families are more likely to be asynchronous on hardware-level)
      // If that doesn't work, try to use different queues from the same family. Most likely these will be synchronous on hardware-level though.
      // Else disable certain queue-types that can't be filled in.

      int numUsedQueues[(uint)CommandListType::NUM] = { 0u };
      for (int i = 0, e = (int)queueFamilyProps.size(); i < e; ++i)
      {
        const VkQueueFamilyProperties& props = queueFamilyProps[i];
        if (props.queueCount == 0u)
          continue;

        if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT && myQueueInfos[(uint) CommandListType::Graphics].myQueueFamilyIndex == -1)
        {
          myQueueInfos[(uint) CommandListType::Graphics].myQueueFamilyIndex = i;
          myQueueInfos[(uint) CommandListType::Graphics].myQueueIndex = 0u;
          ++numUsedQueues[(uint)CommandListType::Graphics];
        }
        else if (props.queueFlags & VK_QUEUE_COMPUTE_BIT && myQueueInfos[(uint)CommandListType::Compute].myQueueFamilyIndex == -1)
        {
          myQueueInfos[(uint)CommandListType::Compute].myQueueFamilyIndex = i;
          myQueueInfos[(uint)CommandListType::Compute].myQueueIndex = 0u;
          ++numUsedQueues[(uint)CommandListType::Compute];
        }
        else if (props.queueFlags & VK_QUEUE_TRANSFER_BIT && myQueueInfos[(uint)CommandListType::DMA].myQueueFamilyIndex == -1)
        {
          myQueueInfos[(uint)CommandListType::DMA].myQueueFamilyIndex = i;
          myQueueInfos[(uint)CommandListType::DMA].myQueueIndex = 0u;
          ++numUsedQueues[(uint)CommandListType::DMA];
        }
      }

      ASSERT(myQueueInfos[(uint)CommandListType::Graphics].myQueueFamilyIndex >= 0, "Could not find a graphics-capable Vulkan queue");

      for (int queueType = (int)CommandListType::Compute; queueType < (int)CommandListType::NUM; ++queueType)
      {
        if (myQueueInfos[queueType].myQueueFamilyIndex < 0)
        {
          for (int higherQueueType = queueType - 1; higherQueueType >= 0; --higherQueueType)
          {
            const int higherFamilyIndex = myQueueInfos[higherQueueType].myQueueFamilyIndex;
            if (higherFamilyIndex < 0)
              continue;

            const VkQueueFamilyProperties& higherFamilyProps = queueFamilyProps[higherFamilyIndex];
            if (numUsedQueues[higherFamilyIndex] < (int) higherFamilyProps.queueCount)
            {
              myQueueInfos[queueType].myQueueFamilyIndex = higherFamilyIndex;
              myQueueInfos[queueType].myQueueIndex = numUsedQueues[higherFamilyIndex]++;
              break;
            }
          }
        }
      }

      LOG("Creating logical Vulkan device...");
      locPrintAvailableDeviceExtensions(myPhysicalDevice);

      const float queuePriority = 1.0f;
      uint numQueuesToCreate = 0u;
      VkDeviceQueueCreateInfo queueCreateInfos[(uint) CommandListType::NUM] = {};
      for (uint queueType = 0u; queueType < (uint)CommandListType::NUM; ++queueType)
      {
        if (numUsedQueues[queueType] > 0)
        {
          queueCreateInfos[numQueuesToCreate].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
          queueCreateInfos[numQueuesToCreate].queueFamilyIndex = myQueueInfos[queueType].myQueueFamilyIndex;
          queueCreateInfos[numQueuesToCreate].queueCount = numUsedQueues[queueType];
          queueCreateInfos[numQueuesToCreate].pQueuePriorities = &queuePriority;
          ++numQueuesToCreate;
        }
      }

      VkDeviceCreateInfo deviceCreateInfo = {};
      deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
      deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
      deviceCreateInfo.queueCreateInfoCount = numQueuesToCreate;
      deviceCreateInfo.pEnabledFeatures = &myPhysicalDeviceFeatures;

      const char* const extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
      deviceCreateInfo.ppEnabledExtensionNames = extensions;
      deviceCreateInfo.enabledExtensionCount = ARRAY_LENGTH(extensions);

      ASSERT_VK_RESULT(vkCreateDevice(myPhysicalDevice, &deviceCreateInfo, nullptr, &myDevice));
    }

    // Init caps
    {
      myCaps.myMaxNumVertexAttributes = myPhysicalDeviceProperties.limits.maxVertexInputAttributes;
      myCaps.myCbufferPlacementAlignment = (uint) myPhysicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
    }
  }
  
  RenderCore_PlatformVk::~RenderCore_PlatformVk()
  {
    Shutdown();
  }

  bool RenderCore_PlatformVk::IsInitialized()
  {
    return myDevice != nullptr;
  }

  bool RenderCore_PlatformVk::InitInternalResources()
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
    return true;
  }

  void RenderCore_PlatformVk::Shutdown()
  {
    vkDestroyInstance(myInstance, nullptr);
    vkDestroyDevice(myDevice, nullptr);
  }

  RenderOutput* RenderCore_PlatformVk::CreateRenderOutput(void* aNativeInstanceHandle, const WindowParameters& someWindowParams)
  {
    return new RenderOutputVk(aNativeInstanceHandle, someWindowParams);
  }

  ShaderCompiler* RenderCore_PlatformVk::CreateShaderCompiler()
  {
    return new ShaderCompilerVk();
  }

  Shader* RenderCore_PlatformVk::CreateShader()
  {
    return new ShaderVk();
  }

  ShaderPipeline* RenderCore_PlatformVk::CreateShaderPipeline()
  {
    return new ShaderPipelineVk();
  }

  Texture* RenderCore_PlatformVk::CreateTexture()
  {
    return new TextureVk();
  }

  GpuBuffer* RenderCore_PlatformVk::CreateBuffer()
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
    return nullptr;
  }

  CommandList* RenderCore_PlatformVk::CreateCommandList(CommandListType aType)
  {
    return new CommandListVk(aType);
  }

  CommandQueue* RenderCore_PlatformVk::CreateCommandQueue(CommandListType aType)
  {
    const QueueInfo& queueInfo = myQueueInfos[(uint)aType];
    if (queueInfo.myQueueFamilyIndex == -1)
      return nullptr;

    return new CommandQueueVk(aType);
  }

  TextureView* RenderCore_PlatformVk::CreateTextureView(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties, const char* aDebugName)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
    return nullptr;
  }

  GpuBufferView* RenderCore_PlatformVk::CreateBufferView(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties, const char* aDebugName)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
    return nullptr;
  }

  GpuQueryHeap* RenderCore_PlatformVk::CreateQueryHeap(GpuQueryType aType, uint aNumQueries)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
    return nullptr;
  }

  uint RenderCore_PlatformVk::GetQueryTypeDataSize(GpuQueryType aType)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
    return 0u;
  }

  float64 RenderCore_PlatformVk::GetGpuTicksToMsFactor(CommandListType aCommandListType)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
    return 1.0;
  }

  VkCommandPool RenderCore_PlatformVk::GetCommandPool(CommandListType aCommandListType)
  {
    // TODO: Don't just create a new pool all the time! Use a caching-scheme like on DX12. However, we first need fences functional on command-queues to detect when its safe to reuse a command pool on a new command list

    VkCommandPool commandPool = nullptr;

    VkCommandPoolCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = myQueueInfos[(uint)aCommandListType].myQueueFamilyIndex;
    ASSERT_VK_RESULT(vkCreateCommandPool(myDevice, &createInfo, nullptr, &commandPool));

    return commandPool;
  }
}
