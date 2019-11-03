#pragma once
#include "RenderCore_Platform.h"

#include "VkPrerequisites.h"
#include "FancyCoreDefines.h"
#include "CommandBufferAllocatorVk.h"

namespace Fancy
{
  class RenderCore_PlatformVk final : public Fancy::RenderCore_Platform
  {
  public:
    static VkFormat ResolveFormat(DataFormat aFormat);
    static VkBlendFactor ResolveBlendFactor(BlendFactor aFactor);
    static VkBlendOp ResolveBlendOp(BlendOp aBlendOp);
    static VkLogicOp ResolveLogicOp(LogicOp aLogicOp);
    static VkStencilOp ResolveStencilOp(StencilOp aStencilOp);
    static VkCompareOp ResolveCompFunc(CompFunc aCompFunc);
    static VkPrimitiveTopology ResolveTopologyType(TopologyType aTopology);
    static VkPolygonMode ResolveFillMode(FillMode aFillMode);
    static VkFrontFace ResolveWindingOrder(WindingOrder aWindingOrder);
    static VkCullModeFlagBits ResolveCullMode(CullMode aCullMode);

    RenderCore_PlatformVk();
    ~RenderCore_PlatformVk() override;
    // Disallow copy and assignment (class contains a list of unique_ptrs)
    RenderCore_PlatformVk(const RenderCore_PlatformVk&) = delete;
    RenderCore_PlatformVk& operator=(const RenderCore_PlatformVk&) = delete;
   
    bool IsInitialized() override;
    bool InitInternalResources() override;
    void Shutdown() override;
    RenderOutput* CreateRenderOutput(void* aNativeInstanceHandle, const WindowParameters& someWindowParams) override;
    ShaderCompiler* CreateShaderCompiler() override;
    Shader* CreateShader() override;
    ShaderPipeline* CreateShaderPipeline() override;
    Texture* CreateTexture() override;
    GpuBuffer* CreateBuffer() override;
    CommandList* CreateCommandList(CommandListType aType) override;
    CommandQueue* CreateCommandQueue(CommandListType aType) override;
    TextureView* CreateTextureView(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties, const char* aDebugName) override;
    GpuBufferView* CreateBufferView(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties, const char* aDebugName) override;
    GpuQueryHeap* CreateQueryHeap(GpuQueryType aType, uint aNumQueries) override;
    uint GetQueryTypeDataSize(GpuQueryType aType) override;
    float64 GetGpuTicksToMsFactor(CommandListType aCommandListType) override;

    VkCommandBuffer GetNewCommandBuffer(CommandListType aCommandListType);
    void ReleaseCommandBuffer(VkCommandBuffer aCommandBuffer, CommandListType aCommandListType, uint64 aCommandBufferDoneFence);

    const VkPhysicalDeviceMemoryProperties& GetPhysicalDeviceMemoryProperties() const { return myPhysicalDeviceMemoryProperties; }
    
    // TODO: Make these members private and add getter-functions where needed
    VkInstance myInstance = nullptr;
    VkPhysicalDevice myPhysicalDevice = nullptr;
    VkDevice myDevice = nullptr;
    VkSurfaceKHR mySurface = nullptr;

    PFN_vkSetDebugUtilsObjectNameEXT VkSetDebugUtilsObjectNameEXT = nullptr;

    struct QueueInfo
    {
      int myQueueFamilyIndex = -1;
      int myQueueIndex = -1;
    };
    QueueInfo myQueueInfos[(uint)CommandListType::NUM];

    VkPhysicalDeviceFeatures myPhysicalDeviceFeatures;
    VkPhysicalDeviceProperties myPhysicalDeviceProperties;

  protected:
    VkPhysicalDeviceMemoryProperties myPhysicalDeviceMemoryProperties;

    UniquePtr<CommandBufferAllocatorVk> myCommandBufferAllocators[(uint)CommandListType::NUM];
  };
}


