#pragma once
#include "RenderCore_Platform.h"

#include "VkPrerequisites.h"
#include "FancyCoreDefines.h"
#include "CommandBufferAllocatorVk.h"
#include "DescriptorPoolAllocatorVk.h"
#include "PipelineLayoutCacheVk.h"
#include "FrameBufferCacheVk.h"
#include "RenderPassCacheVk.h"
#include "PipelineStateCacheVk.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct ResourceBarrierInfoVk
  {
    VkPipelineStageFlags myStageMask;
    VkAccessFlags myAccessMask;
    VkImageLayout myImageLayout;
  };
//---------------------------------------------------------------------------//
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
    // static ResourceBarrierInfoVk ResolveResourceState(GpuResourceState aResourceState);
    static VkImageAspectFlags ResolveAspectMask(uint aFirstPlaneIndex, uint aNumPlanes, DataFormat aFormat);
    static VkImageSubresourceRange ResolveSubresourceRange(const SubresourceRange& aRange, DataFormat aFormat);
    static VkImageType ResolveImageResourceDimension(GpuResourceDimension aDimension, bool& isArray, bool& isCubeMap);
    static VkQueryType ResolveQueryType(GpuQueryType aType);
    static VkVertexInputRate ResolveVertexInputRate(VertexInputRate aRate);
    static uint ImageLayoutToFlag(VkImageLayout aLayout);

    RenderCore_PlatformVk();
    ~RenderCore_PlatformVk() override;
    // Disallow copy and assignment (class contains a list of unique_ptrs)
    RenderCore_PlatformVk(const RenderCore_PlatformVk&) = delete;
    RenderCore_PlatformVk& operator=(const RenderCore_PlatformVk&) = delete;
   
    bool IsInitialized() override;
    bool InitInternalResources() override;
    void Shutdown() override;
    void BeginFrame() override;

    RenderOutput* CreateRenderOutput(void* aNativeInstanceHandle, const WindowParameters& someWindowParams) override;
    ShaderCompiler* CreateShaderCompiler() override;
    Shader* CreateShader() override;
    ShaderPipeline* CreateShaderPipeline() override;
    Texture* CreateTexture() override;
    GpuBuffer* CreateBuffer() override;
    TextureSampler* CreateTextureSampler(const TextureSamplerProperties& someProperties) override;
    CommandList* CreateCommandList(CommandListType aType) override;
    CommandQueue* CreateCommandQueue(CommandListType aType) override;
    TextureView* CreateTextureView(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties, const char* aDebugName) override;
    GpuBufferView* CreateBufferView(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties, const char* aDebugName) override;
    GpuQueryHeap* CreateQueryHeap(GpuQueryType aType, uint aNumQueries) override;
    uint GetQueryTypeDataSize(GpuQueryType aType) override;
    float64 GetGpuTicksToMsFactor(CommandListType aCommandListType) override;

    VkCommandBuffer GetNewCommandBuffer(CommandListType aCommandListType);
    void ReleaseCommandBuffer(VkCommandBuffer aCommandBuffer, CommandListType aCommandListType, uint64 aCommandBufferDoneFence);

    VkDescriptorPool AllocateDescriptorPool();
    void FreeDescriptorPool(VkDescriptorPool aDescriptorPool, uint64 aFence);

    uint FindMemoryTypeIndex(const VkMemoryRequirements& someMemoryRequirements, VkMemoryPropertyFlags someMemPropertyFlags);
    const VkPhysicalDeviceMemoryProperties& GetPhysicalDeviceMemoryProperties() const { return myPhysicalDeviceMemoryProperties; }

    void ReleaseTempBufferView(VkBufferView aBufferView, uint64 aFence);
    void DestroyTempBufferViews();

    struct QueueInfo
    {
      uint myQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      uint myQueueIndex = UINT_MAX;
    };

    const QueueInfo& GetQueueInfo(CommandListType aCommandListType) { return myQueueInfos[(uint)aCommandListType]; }

    VkDevice GetDevice() const { return myDevice; }

    PipelineLayoutCacheVk& GetPipelineLayoutCache() { return myPipelineLayoutCache; }
    FrameBufferCacheVk& GetFrameBufferCache() { return myFrameBufferCache; }
    RenderPassCacheVk& GetRenderPassCache() { return myRenderPassCache; }
    PipelineStateCacheVk& GetPipelineStateCache() { return myPipelineStateCache; }

    // TODO: Make these members private and add getter-functions where needed
    VkInstance myInstance = nullptr;
    VkPhysicalDevice myPhysicalDevice = nullptr;
    VkDevice myDevice = nullptr;
    VkSurfaceKHR mySurface = nullptr;

    PFN_vkSetDebugUtilsObjectNameEXT VkSetDebugUtilsObjectNameEXT = nullptr;

    QueueInfo myQueueInfos[(uint)CommandListType::NUM];

    VkPhysicalDeviceFeatures myPhysicalDeviceFeatures;
    VkPhysicalDeviceProperties myPhysicalDeviceProperties;

  protected:
    VkPhysicalDeviceMemoryProperties myPhysicalDeviceMemoryProperties;

    UniquePtr<CommandBufferAllocatorVk> myCommandBufferAllocators[(uint)CommandListType::NUM];
    UniquePtr<DescriptorPoolAllocatorVk> myDescriptorPoolAllocator;

    eastl::fixed_vector<eastl::pair<VkBufferView, uint64>, 64> myTempBufferViews;

    PipelineLayoutCacheVk myPipelineLayoutCache;
    FrameBufferCacheVk myFrameBufferCache;
    RenderPassCacheVk myRenderPassCache;
    PipelineStateCacheVk myPipelineStateCache;

    float64 myTimestampTicksToMsFactor = 0.0f;
  };
}

#endif