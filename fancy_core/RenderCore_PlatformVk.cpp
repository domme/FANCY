#include "fancy_core_precompile.h"

#include "RenderCore_PlatformVk.h"
#include "RendererPrerequisites.h"
#include "RenderOutputVk.h"
#include "CommandQueueVk.h"

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

      const char* const extensions[] = { VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_win32_surface", VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
      createInfo.enabledExtensionCount = ARRAY_LENGTH(extensions);
      createInfo.ppEnabledExtensionNames = extensions;

      const char* const layers[] = { "VK_LAYER_KHRONOS_validation" };
      createInfo.enabledLayerCount = ARRAY_LENGTH(layers);
      createInfo.ppEnabledLayerNames = layers;

      ASSERT_VK_RESULT(vkCreateInstance(&createInfo, nullptr, &myInstance));
      LOG("Initialized Vulkan instance");
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
      
      for (int& idx : myQueueIndices)
        idx = -1;

      for (int i = 0, e = (int)queueFamilyProps.size(); i < e; ++i)
      {
        const VkQueueFamilyProperties& props = queueFamilyProps[i];
        if (props.queueCount == 0u)
          continue;

        if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT)
          myQueueIndices[(uint)CommandListType::Graphics] = i;
        if (props.queueFlags & VK_QUEUE_COMPUTE_BIT)
          myQueueIndices[(uint)CommandListType::Compute] = i;
        if (props.queueFlags & VK_QUEUE_TRANSFER_BIT)
          myQueueIndices[(uint)CommandListType::DMA] = i;
      }

      ASSERT(myQueueIndices[(uint)CommandListType::Graphics] >= 0, "Vulkan device doesn't support graphics queue");
      ASSERT(myQueueIndices[(uint)CommandListType::Compute] >= 0, "Vulkan device doesn't support compute queue");

      LOG("Creating logical Vulkan device...");
      locPrintAvailableDeviceExtensions(myPhysicalDevice);

      VkDeviceQueueCreateInfo queueCreateInfos[] = { {}, {} };
      queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfos[0].queueFamilyIndex = myQueueIndices[(uint)CommandListType::Graphics];
      queueCreateInfos[0].queueCount = 1;
      float queuePriority = 1.0f;
      queueCreateInfos[0].pQueuePriorities = &queuePriority;

      queueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfos[1].queueFamilyIndex = myQueueIndices[(uint)CommandListType::Compute];
      queueCreateInfos[1].queueCount = 1;
      queueCreateInfos[1].pQueuePriorities = &queuePriority;

      VkDeviceCreateInfo deviceCreateInfo = {};
      deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
      deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
      deviceCreateInfo.queueCreateInfoCount = ARRAY_LENGTH(queueCreateInfos);
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
    ASSERT(false, "Not implemented");
    return true;
  }

  void RenderCore_PlatformVk::Shutdown()
  {
    vkDestroyInstance(myInstance, nullptr);
    vkDestroyDevice(myDevice, nullptr);
  }

  RenderOutput* RenderCore_PlatformVk::CreateRenderOutput(HINSTANCE aNativeInstanceHandle, const WindowParameters& someWindowParams)
  {
    return new RenderOutputVk(aNativeInstanceHandle, someWindowParams);
  }

  GpuProgramCompiler* RenderCore_PlatformVk::CreateShaderCompiler()
  {
    ASSERT(false, "Not implemented");
    return nullptr;
  }

  GpuProgram* RenderCore_PlatformVk::CreateGpuProgram()
  {
    return nullptr;
  }

  Texture* RenderCore_PlatformVk::CreateTexture()
  {
    ASSERT(false, "Not implemented");
    return nullptr;
  }

  GpuBuffer* RenderCore_PlatformVk::CreateBuffer()
  {
    ASSERT(false, "Not implemented");
    return nullptr;
  }

  CommandList* RenderCore_PlatformVk::CreateContext(CommandListType aType)
  {
    ASSERT(false, "Not implemented");
    return nullptr;
  }

  CommandQueue* RenderCore_PlatformVk::CreateCommandQueue(CommandListType aType)
  {
    return new CommandQueueVk(aType);
  }

  TextureView* RenderCore_PlatformVk::CreateTextureView(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties, const char* aDebugName)
  {
    ASSERT(false, "Not implemented");
    return nullptr;
  }

  GpuBufferView* RenderCore_PlatformVk::CreateBufferView(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties, const char* aDebugName)
  {
    ASSERT(false, "Not implemented");
    return nullptr;
  }

  GpuQueryHeap* RenderCore_PlatformVk::CreateQueryHeap(GpuQueryType aType, uint aNumQueries)
  {
    ASSERT(false, "Not implemented");
    return nullptr;
  }

  uint RenderCore_PlatformVk::GetQueryTypeDataSize(GpuQueryType aType)
  {
    ASSERT(false, "Not implemented");
    return 0u;
  }

  float64 RenderCore_PlatformVk::GetGpuTicksToMsFactor(CommandListType aCommandListType)
  {
    ASSERT(false, "Not implemented");
    return 1.0;
  }
}
