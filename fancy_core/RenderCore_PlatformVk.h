#pragma once
#include "RenderCore_Platform.h"

#include "VkPrerequisites.h"
#include "FancyCoreDefines.h"

namespace Fancy
{
  class RenderCore_PlatformVk final : public Fancy::RenderCore_Platform
  {
  public:
    RenderCore_PlatformVk();
    ~RenderCore_PlatformVk() override;
    // Disallow copy and assignment (class contains a list of unique_ptrs)
    RenderCore_PlatformVk(const RenderCore_PlatformVk&) = delete;
    RenderCore_PlatformVk& operator=(const RenderCore_PlatformVk&) = delete;
   
    bool IsInitialized() override;
    bool InitInternalResources() override;
    void Shutdown() override;
    RenderOutput* CreateRenderOutput(void* aNativeInstanceHandle, const WindowParameters& someWindowParams) override;
    GpuProgramCompiler* CreateShaderCompiler() override;
    GpuProgram* CreateGpuProgram() override;
    Texture* CreateTexture() override;
    GpuBuffer* CreateBuffer() override;
    CommandList* CreateContext(CommandListType aType) override;
    CommandQueue* CreateCommandQueue(CommandListType aType) override;
    TextureView* CreateTextureView(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties,
      const char* aDebugName) override;
    GpuBufferView* CreateBufferView(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties,
      const char* aDebugName) override;
    GpuQueryHeap* CreateQueryHeap(GpuQueryType aType, uint aNumQueries) override;
    uint GetQueryTypeDataSize(GpuQueryType aType) override;
    float64 GetGpuTicksToMsFactor(CommandListType aCommandListType) override;
    
    VkInstance myInstance = nullptr;
    VkPhysicalDevice myPhysicalDevice = nullptr;
    VkDevice myDevice = nullptr;
    int myQueueIndices[(uint)CommandListType::NUM] = { 0u };

    VkPhysicalDeviceFeatures myPhysicalDeviceFeatures;
    VkPhysicalDeviceProperties myPhysicalDeviceProperties;
  };
}


