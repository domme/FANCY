#pragma once
#include "CommandList.h"
#include "MathIncludes.h"
#include "VkPrerequisites.h"
#include <glm/detail/type_mat.hpp>

#if FANCY_ENABLE_VK

namespace Fancy
{
  struct ShaderResourceInfoVk;

  class CommandListVk : public CommandList
  {
    friend class CommandQueueVk;

  public:
    struct BufferMemoryBarrierData
    {
      VkBuffer myBuffer = nullptr;
      VkAccessFlags mySrcAccessMask = 0;
      VkAccessFlags myDstAccessMask = 0;
      uint mySrcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      uint myDstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    };

    struct ImageMemoryBarrierData
    {
      SubresourceRange mySubresourceRange;
      VkImage myImage = nullptr;
      DataFormat myFormat = DataFormat::UNKNOWN;
      VkAccessFlags mySrcAccessMask = 0;
      VkAccessFlags myDstAccessMask = 0;
      VkImageLayout mySrcLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      VkImageLayout myDstLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      uint mySrcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      uint myDstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    };

    CommandListVk(CommandListType aType);
    ~CommandListVk();

    void ClearRenderTarget(TextureView* aTextureView, const float* aColor) override;
    void ClearDepthStencilTarget(TextureView* aTextureView, float aDepthClear, uint8 aStencilClear, uint someClearFlags) override;
    void CopyResource(GpuResource* aDstResource, GpuResource* aSrcResource) override;
    void CopyBuffer(const GpuBuffer* aDstBuffer, uint64 aDstOffset, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset, uint64 aSize) override;
    void CopyTextureToBuffer(const GpuBuffer* aDstBuffer, uint64 aDstOffset, const Texture* aSrcTexture, const SubresourceLocation& aSrcSubresource, const TextureRegion& aSrcRegion) override;
    void CopyTexture(const Texture* aDstTexture, const SubresourceLocation& aDstSubresource, const TextureRegion& aDstRegion, const Texture* aSrcTexture, const SubresourceLocation& aSrcSubresource, const TextureRegion& aSrcRegion) override;
    void CopyBufferToTexture(const Texture* aDstTexture, const SubresourceLocation& aDstSubresource, const TextureRegion& aDstRegion, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset) override;

    void PostExecute(uint64 aFenceVal) override;
    void PreBegin() override;
    void FlushBarriers() override;
    void SetShaderPipeline(const SharedPtr<ShaderPipeline>& aShaderPipeline) override;
    void BindVertexBuffer(const GpuBuffer* aBuffer, uint aVertexSize, uint64 anOffset, uint64 aSize) override;
    void BindIndexBuffer(const GpuBuffer* aBuffer, uint anIndexSize, uint64 anOffset, uint64 aSize) override;
    void Render(uint aNumIndicesPerInstance, uint aNumInstances, uint aStartIndex, uint aBaseVertex, uint aStartInstance) override;
    void UpdateTextureData(const Texture* aDstTexture, const SubresourceRange& aSubresourceRange, const TextureSubData* someDatas, uint aNumDatas /*, const TextureRegion* someRegions = nullptr */) override;

    void BindResourceView(const GpuResourceView* aView, uint64 aNameHash, uint anArrayIndex = 0u) override;
    void BindBuffer(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someViewProperties, uint64 aNameHash, uint anArrayIndex = 0u) override;
    void BindSampler(const TextureSampler* aSampler, uint64 aNameHash, uint anArrayIndex = 0u) override;
    
    GpuQuery BeginQuery(GpuQueryType aType) override;
    void EndQuery(const GpuQuery& aQuery) override;
    GpuQuery InsertTimestamp() override;
    void CopyQueryDataToBuffer(const GpuQueryHeap* aQueryHeap, const GpuBuffer* aBuffer, uint aFirstQueryIndex, uint aNumQueries, uint64 aBufferOffset) override;

    void TransitionResource(const GpuResource* aResource, const SubresourceRange& aSubresourceRange, ResourceTransition aTransition) override;
    void ResourceUAVbarrier(const GpuResource** someResources, uint aNumResources) override;

    void Close() override;
    
    void SetComputeProgram(const Shader* aProgram) override;
    void Dispatch(const glm::int3& aNumThreads) override;

    VkCommandBuffer GetCommandBuffer() const { return myCommandBuffer; }

    void TrackResourceTransition(const GpuResource* aResource, VkAccessFlags aNewAccessFlags, VkImageLayout aNewImageLayout, VkPipelineStageFlags aNewPipelineStageFlags, bool aIsSharedReadState = false);
    void TrackSubresourceTransition(const GpuResource* aResource, const SubresourceRange& aSubresourceRange, VkAccessFlags aNewAccessFlags, VkImageLayout aNewImageLayout, VkPipelineStageFlags aNewPipelineStageFlags, bool aIsSharedReadState = false);
    void AddBarrier(const BufferMemoryBarrierData& aBarrier);
    void AddBarrier(const ImageMemoryBarrierData& aBarrier);
    
  protected:
    bool FindShaderResourceInfo(uint64 aNameHash, ShaderResourceInfoVk& aResourceInfoOut) const;
    void ClearResourceState();
    void BindInternal(const ShaderResourceInfoVk &aResourceInfo,
      uint anArrayIndex,
      VkBufferView aBufferView,
      VkBuffer aBuffer,
      uint64 aBufferOffset,
      uint64 aBufferSize,
      VkImageView anImageView,
      VkImageLayout anImageLayout,
      VkSampler aSampler);

    bool ValidateSubresourceTransition(const GpuResource* aResource, uint aSubresourceIndex, VkAccessFlags aDstAccess, VkImageLayout aDstImageLayout);

    void ApplyViewportAndClipRect();
    void ApplyRenderTargets();
    void ApplyGraphicsPipelineState();
    void ApplyComputePipelineState();
    void ApplyResourceState();

    VkDescriptorSet CreateDescriptorSet(VkDescriptorSetLayout aLayout);

    void BeginCommandBuffer();

    static std::unordered_map<uint64, VkPipeline> ourPipelineCache;
    static std::unordered_map<uint64, VkRenderPass> ourRenderpassCache;
    static std::unordered_map<uint64, VkFramebuffer> ourFramebufferCache;

    VkCommandBuffer myCommandBuffer;
    VkRenderPass myRenderPass;
    VkFramebuffer myFramebuffer;
    glm::uvec2 myFramebufferRes;

    struct ResourceState
    {
      struct DescriptorRange
      {
        uint myBindingInSet = UINT_MAX;
        VkDescriptorType myType = VK_DESCRIPTOR_TYPE_MAX_ENUM;
        DynamicArray<VkDescriptorImageInfo> myImageInfos;
        DynamicArray<VkDescriptorBufferInfo> myBufferInfos;
        DynamicArray<VkBufferView> myTexelBufferViews;
      };

      struct DescriptorSet
      {
        uint mySet;
        VkDescriptorSetLayout myLayout;
        StaticArray<DescriptorRange, 64> myRanges;
      };

      StaticArray<std::pair<VkBufferView, uint64>, 64> myTempBufferViews;
      StaticArray<DescriptorSet, 16> myDescriptorSets;
    };

    ResourceState myResourceState;

    struct SubresourceHazardData
    {
      VkAccessFlags myFirstDstAccessFlags = 0u;
      VkImageLayout myFirstDstImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      VkAccessFlags myAccessFlags = 0u;
      VkImageLayout myImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      bool myWasWritten = false;
      bool myWasUsed = false;
      bool myIsSharedReadState = false;
    };
    struct LocalHazardData
    {
      DynamicArray<SubresourceHazardData> mySubresources;
    };
    std::map<const GpuResource*, LocalHazardData> myLocalHazardData;
    
    StaticArray<VkDescriptorPool, 16> myUsedDescriptorPools;
    StaticArray<BufferMemoryBarrierData, kNumCachedBarriers> myPendingBufferBarriers;
    StaticArray<ImageMemoryBarrierData, kNumCachedBarriers> myPendingImageBarriers;

    VkPipelineStageFlags myPendingBarrierSrcStageMask;
    VkPipelineStageFlags myPendingBarrierDstStageMask;
  };
}

#endif