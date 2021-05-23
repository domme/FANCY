#pragma once
#include "CommandList.h"
#include "MathIncludes.h"
#include "VkPrerequisites.h"
#include "FancyCoreDefines.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  struct ShaderResourceInfoVk;
  class ShaderPipelineVk;
  struct PipelineLayoutBindingsVk;

  class CommandListVk : public CommandList
  {
    friend class CommandQueueVk;

  public:
    struct BufferMemoryBarrierData
    {
      VkBuffer myBuffer = nullptr;
      uint64 myBufferSize = 0ull;
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

    void BindVertexBuffers(const GpuBuffer** someBuffers, uint64* someOffsets, uint64* someSizes, uint aNumBuffers) override;
    void BindIndexBuffer(const GpuBuffer* aBuffer, uint anIndexSize, uint64 anOffset, uint64 aSize) override;
    void Render(uint aNumIndicesPerInstance, uint aNumInstances, uint aStartIndex, uint aBaseVertex, uint aStartInstance) override;
    void UpdateTextureData(const Texture* aDstTexture, const SubresourceRange& aSubresourceRange, const TextureSubData* someDatas, uint aNumDatas /*, const TextureRegion* someRegions = nullptr */) override;

    void BindResourceView(const GpuResourceView* aView, uint64 aNameHash, uint anArrayIndex = 0u) override;
    void BindResourceViewSet(const GpuResourceViewSet* aSet, uint aSetOrTableIndex) override;
    void BindLocalBuffer(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someViewProperties, uint64 aNameHash, uint anArrayIndex = 0u) override;
    void BindSampler(const TextureSampler* aSampler, uint64 aNameHash, uint anArrayIndex = 0u) override;
    
    GpuQuery BeginQuery(GpuQueryType aType) override;
    void EndQuery(const GpuQuery& aQuery) override;
    GpuQuery InsertTimestamp() override;
    void CopyQueryDataToBuffer(const GpuQueryHeap* aQueryHeap, const GpuBuffer* aBuffer, uint aFirstQueryIndex, uint aNumQueries, uint64 aBufferOffset) override;

    void TransitionResource(const GpuResource* aResource, const SubresourceRange& aSubresourceRange, ResourceTransition aTransition, uint someUsageFlags = 0u) override;
    void ResourceUAVbarrier(const GpuResource** someResources, uint aNumResources) override;

    void Close() override;
    
    void Dispatch(const glm::int3& aNumThreads) override;

    VkCommandBuffer GetCommandBuffer() const { return myCommandBuffer; }

    void TrackResourceTransition(const GpuResource* aResource, VkAccessFlags aNewAccessFlags, VkImageLayout aNewImageLayout, VkPipelineStageFlags aNewPipelineStageFlags, bool aToSharedReadState = false);
    void TrackSubresourceTransition(const GpuResource* aResource, const SubresourceRange& aSubresourceRange, VkAccessFlags aNewAccessFlags, VkImageLayout aNewImageLayout, VkPipelineStageFlags aNewPipelineStageFlags, bool aToSharedReadState = false);
    void AddBarrier(const BufferMemoryBarrierData& aBarrier);
    void AddBarrier(const ImageMemoryBarrierData& aBarrier);
    
  protected:
    void ClearResourceBindings();
    void SetShaderPipelineInternal(const ShaderPipeline* aPipeline, bool& aHasPipelineChangedOut) override;

    const ShaderPipelineVk* GetShaderPipeline() const;

    const ShaderResourceInfoVk* FindShaderResourceInfo(uint64 aNameHash) const;
    void BindInternal(const ShaderResourceInfoVk &aResourceInfo,
      uint anArrayIndex,
      const eastl::optional<VkDescriptorBufferInfo>& aDescriptorBufferInfo,
      const eastl::optional<VkDescriptorImageInfo>& aDescriptorImageInfo,
      const eastl::optional<VkBufferView>& aBufferView);

    bool ValidateSubresourceTransition(const GpuResource* aResource, uint aSubresourceIndex, VkAccessFlags aDstAccess, VkImageLayout aDstImageLayout);

    void ApplyViewportAndClipRect();
    void ApplyRenderTargets();
    void ApplyGraphicsPipelineState();
    void ApplyComputePipelineState();
    void ApplyResourceBindings();

    VkDescriptorSet CreateDescriptorSet(VkDescriptorSetLayout aLayout);

    void BeginCommandBuffer();

    VkCommandBuffer myCommandBuffer;
    VkRenderPass myRenderPass;
    VkFramebuffer myFramebuffer;
    glm::uvec2 myFramebufferRes;
        
    VkPipelineLayout myPipelineLayout = nullptr;
    UniquePtr<PipelineLayoutBindingsVk> myPipelineLayoutBindings;
    eastl::fixed_vector<eastl::pair<VkBufferView, uint64>, 32> myTempBufferViews;

    struct SubresourceHazardData
    {
      SubresourceHazardData()
        : myWasWritten(false)
        , myWasUsed(false)
        , myIsSharedReadState(false)
      { }

      VkAccessFlags myFirstDstAccessFlags = 0u;
      VkImageLayout myFirstDstImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      VkAccessFlags myAccessFlags = 0u;
      VkImageLayout myImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      uint myWasWritten : 1;
      uint myWasUsed : 1;
      uint myIsSharedReadState : 1;
    };
    struct LocalHazardData
    {
      eastl::fixed_vector<SubresourceHazardData, 16> mySubresources;
    };
    eastl::fixed_hash_map<const GpuResource*, LocalHazardData, kNumExpectedResourcesPerDispatch> myLocalHazardData;
    
    eastl::fixed_vector<VkDescriptorPool, 16> myUsedDescriptorPools;
    eastl::fixed_vector<BufferMemoryBarrierData, kNumCachedBarriers> myPendingBufferBarriers;
    eastl::fixed_vector<ImageMemoryBarrierData, kNumCachedBarriers> myPendingImageBarriers;

    VkPipelineStageFlags myPendingBarrierSrcStageMask;
    VkPipelineStageFlags myPendingBarrierDstStageMask;
  };
}

#endif