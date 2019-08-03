#include "fancy_core_precompile.h"

#include "RenderCore_PlatformVk.h"
#include "RendererPrerequisites.h"

namespace Fancy
{
  namespace
  {
    void locPrintAvailableExtensions()
    {
      uint numAvailableExtensions = 0u;
      vkEnumerateInstanceExtensionProperties(nullptr, &numAvailableExtensions, nullptr);

      DynamicArray<VkExtensionProperties> availableExtensionProperties;
      availableExtensionProperties.resize(numAvailableExtensions);
      vkEnumerateInstanceExtensionProperties(nullptr, &numAvailableExtensions, availableExtensionProperties.data());

      LOG_INFO("Available Vulkan extensions:");
      for (const VkExtensionProperties& prop : availableExtensionProperties)
        LOG_INFO("%s (spec version %d)", prop.extensionName, prop.specVersion);
    }

    void locPrintAvailableLayers()
    {
      uint numAvailableLayers = 0u;
      vkEnumerateInstanceLayerProperties(&numAvailableLayers, nullptr);

      DynamicArray<VkLayerProperties> availableLayerProperties;
      availableLayerProperties.resize(numAvailableLayers);
      vkEnumerateInstanceLayerProperties(&numAvailableLayers, availableLayerProperties.data());

      LOG_INFO("Available Vulkan Layers:");
      for (const VkLayerProperties& prop : availableLayerProperties)
        LOG_INFO("%s (spec version %d, implementation version %d)", prop.layerName, prop.specVersion, prop.implementationVersion);
    }
  }

  RenderCore_PlatformVk::RenderCore_PlatformVk()
    : RenderCore_Platform(RenderPlatformType::VULKAN)
  {
    LOG_INFO("Initializing Vulkan device...");
    locPrintAvailableExtensions();
    locPrintAvailableLayers();

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Fancy";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    const char* const extensions[] = { VK_KHR_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
    createInfo.enabledExtensionCount = ARRAY_LENGTH(extensions);
    createInfo.ppEnabledExtensionNames = extensions;

    const char* const layers[] = { "VK_LAYER_KHRONOS_validation" };
    createInfo.enabledLayerCount = ARRAY_LENGTH(layers);
    createInfo.ppEnabledLayerNames = layers;

    ASSERT_VK_RESULT(vkCreateInstance(&createInfo, nullptr, &myInstance));
    LOG_INFO("Initialized Vulkan device");

  }
  
  RenderCore_PlatformVk::~RenderCore_PlatformVk()
  {
    vkDestroyInstance(myInstance, nullptr);
  }

  bool RenderCore_PlatformVk::IsInitialized()
  {
    return true;
  }

  bool RenderCore_PlatformVk::InitInternalResources()
  {
    return true;
  }

  void RenderCore_PlatformVk::InitCaps()
  {
  }

  void RenderCore_PlatformVk::Shutdown()
  {
  }

  RenderOutput* RenderCore_PlatformVk::CreateRenderOutput(void* aNativeInstanceHandle,
    const WindowParameters& someWindowParams)
  {
    return nullptr;
  }

  GpuProgramCompiler* RenderCore_PlatformVk::CreateShaderCompiler()
  {
    return nullptr;
  }

  GpuProgram* RenderCore_PlatformVk::CreateGpuProgram()
  {
    return nullptr;
  }

  Texture* RenderCore_PlatformVk::CreateTexture()
  {
    return nullptr;
  }

  GpuBuffer* RenderCore_PlatformVk::CreateBuffer()
  {
    return nullptr;
  }

  CommandList* RenderCore_PlatformVk::CreateContext(CommandListType aType)
  {
    return nullptr;
  }

  CommandQueue* RenderCore_PlatformVk::CreateCommandQueue(CommandListType aType)
  {
    return nullptr;
  }

  TextureView* RenderCore_PlatformVk::CreateTextureView(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties, const char* aDebugName)
  {
    return nullptr;
  }

  GpuBufferView* RenderCore_PlatformVk::CreateBufferView(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties, const char* aDebugName)
  {
    return nullptr;
  }

  GpuQueryHeap* RenderCore_PlatformVk::CreateQueryHeap(GpuQueryType aType, uint aNumQueries)
  {
    return nullptr;
  }

  uint RenderCore_PlatformVk::GetQueryTypeDataSize(GpuQueryType aType)
  {
    return 0u;
  }

  float64 RenderCore_PlatformVk::GetGpuTicksToMsFactor(CommandListType aCommandListType)
  {
    return 1.0;
  }
}
