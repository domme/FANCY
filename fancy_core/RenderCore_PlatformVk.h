#pragma once
#include "RenderCore_Platform.h"

#include "VkPrerequisites.h"
#include "FancyCoreDefines.h"
#include "CommandBufferAllocatorVk.h"
#include "DescriptorPoolAllocatorVk.h"
#include "FrameBufferCacheVk.h"
#include "GlobalDescriptorAllocation.h"
#include "RenderPassCacheVk.h"
#include "PipelineStateCacheVk.h"

#if FANCY_ENABLE_VK

struct VkExt
{
  static PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
  static PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
  static PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
  static PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
  static PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
  static PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR;
  static PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
  static PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;
  static PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;
};

namespace Fancy
{
  class GlobalDescriptorSetVk;
  struct PipelineLayoutVk;

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
    enum Consts
    {
      MAX_DESCRIPTOR_ARRAY_SIZE = 1000u 
    };
    
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
    static VkImageLayout ResolveImageLayout(VkImageLayout aLayout, const GpuResource* aResource, const SubresourceRange& aSubresourceRange);
    static VkDescriptorType GetDescriptorType(const GpuResourceView* aView);
    static VkDescriptorType GetDescriptorType(GlobalResourceType aResourceType);
    static VkAccelerationStructureTypeKHR GetRtAccelerationStructureType(RtAccelerationStructureType aType);
    static VkGeometryTypeKHR GetRaytracingBVHGeometryType(RtAccelerationStructureGeometryType aType);

    RenderCore_PlatformVk(const RenderPlatformProperties& someProperties);
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
    RtAccelerationStructure* CreateRtBottomLevelAccelerationStructure(const RtAccelerationStructureGeometryData* someGeometries, uint aNumGeometries, uint aSomeFlags = 0, const char* aName = nullptr) override;
    RtAccelerationStructure* CreateRtTopLevelAccelerationStructure(const RtAccelerationStructureInstanceData* someInstances, uint aNumInstances, uint someFlags = 0, const char* aName = nullptr) override;
    RtPipelineState* CreateRtPipelineState(const RtPipelineStateProperties& someProps) override;
    
    GpuQueryHeap* CreateQueryHeap(GpuQueryType aType, uint aNumQueries) override;
    uint GetQueryTypeDataSize(GpuQueryType aType) override;
    float64 GetGpuTicksToMsFactor(CommandListType aCommandListType) override;

    VkCommandBuffer GetNewCommandBuffer(CommandListType aCommandListType);
    void ReleaseCommandBuffer(VkCommandBuffer aCommandBuffer, CommandListType aCommandListType, uint64 aCommandBufferDoneFence);
  
    VkDescriptorPool AllocateDescriptorPool();
    void FreeDescriptorPool(VkDescriptorPool aDescriptorPool, uint64 aFence);

    GlobalDescriptorAllocation AllocateAndWriteGlobalRTASDescriptor(VkAccelerationStructureKHR anAccelerationStructure, const char* aDebugName = nullptr);
    GlobalDescriptorAllocation AllocateAndWriteGlobalResourceDescriptor(GlobalResourceType aType, const VkDescriptorImageInfo& anImageInfo, const char* aDebugName = nullptr);
    GlobalDescriptorAllocation AllocateAndWriteGlobalResourceDescriptor(GlobalResourceType aType, const VkDescriptorBufferInfo& aBufferInfo, const char* aDebugName = nullptr);
    void FreeGlobalResourceDescriptor(const GlobalDescriptorAllocation& aDescriptor);

    uint FindMemoryTypeIndex(const VkMemoryRequirements& someMemoryRequirements, VkMemoryPropertyFlags someMemPropertyFlags);
    const VkPhysicalDeviceMemoryProperties& GetPhysicalDeviceMemoryProperties() const { return myPhysicalDeviceMemoryProperties; }

    struct QueueInfo
    {
      uint myQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      uint myQueueIndex = UINT_MAX;
    };

    const QueueInfo& GetQueueInfo(CommandListType aCommandListType) { return myQueueInfos[(uint)aCommandListType]; }

    VkDevice GetDevice() const { return myDevice; }

    FrameBufferCacheVk& GetFrameBufferCache() { return myFrameBufferCache; }
    RenderPassCacheVk& GetRenderPassCache() { return myRenderPassCache; }
    PipelineStateCacheVk& GetPipelineStateCache() { return myPipelineStateCache; }
    PipelineLayoutVk* GetPipelineLayout() const { return myPipelineLayout.get(); }
    GlobalDescriptorSetVk* GetGlobalDescriptorSet() const { return myGlobalDescriptorSet.get(); }
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
    eastl::vector<eastl::string> mySupportedDeviceExtensions;

    VkPhysicalDeviceMemoryProperties myPhysicalDeviceMemoryProperties;

    UniquePtr<CommandBufferAllocatorVk> myCommandBufferAllocators[(uint)CommandListType::NUM];
    UniquePtr<DescriptorPoolAllocatorVk> myDescriptorPoolAllocator;
    UniquePtr<PipelineLayoutVk> myPipelineLayout;
    UniquePtr<GlobalDescriptorSetVk> myGlobalDescriptorSet;

    FrameBufferCacheVk myFrameBufferCache;
    RenderPassCacheVk myRenderPassCache;
    PipelineStateCacheVk myPipelineStateCache;

    float64 myTimestampTicksToMsFactor = 0.0f;
  };
}

#endif