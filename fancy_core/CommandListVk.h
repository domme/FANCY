#pragma once
#include "CommandList.h"
#include "MathIncludes.h"
#include "VkPrerequisites.h"
#include <glm/detail/type_mat.hpp>

namespace Fancy
{
  struct ShaderResourceInfoVk;

  class CommandListVk : public CommandList
  {
  public:
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

    void BindResourceView(const GpuResourceView* aView, uint64 aNameHash) override;
    void BindBuffer(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someViewProperties, uint64 aNameHash) override;
    void BindSampler(const TextureSampler* aSampler, uint64 aNameHash) override;
    
    GpuQuery BeginQuery(GpuQueryType aType) override;
    void EndQuery(const GpuQuery& aQuery) override;
    GpuQuery InsertTimestamp() override;
    void CopyQueryDataToBuffer(const GpuQueryHeap* aQueryHeap, const GpuBuffer* aBuffer, uint aFirstQueryIndex, uint aNumQueries, uint64 aBufferOffset) override;

    void ResourceUAVbarrier(const GpuResource** someResources, uint aNumResources) override;

    void Close() override;
    
    void SetComputeProgram(const Shader* aProgram) override;
    void Dispatch(const glm::int3& aNumThreads) override;

    VkCommandBuffer GetCommandBuffer() const { return myCommandBuffer; }

    bool SubresourceBarrierInternal(
      const GpuResource* aResource,
      const SubresourceRange& aSubresourceRange,
      VkAccessFlags aSrcAccessMask,
      VkAccessFlags aDstAccessMask,
      VkPipelineStageFlags aSrcStageMask,
      VkPipelineStageFlags aDstStageMask,
      VkImageLayout aSrcImageLayout,
      VkImageLayout aDstImageLayout,
      CommandListType aSrcQueue,
      CommandListType aDstQueue
    );
    
  protected:
    bool FindShaderResourceInfo(uint64 aNameHash, ShaderResourceInfoVk& aResourceInfoOut) const;

    bool SubresourceBarrierInternal(
      const GpuResource* aResource,
      const SubresourceRange& aSubresourceRange,
      GpuResourceState aSrcState,
      GpuResourceState aDstState,
      CommandListType aSrcQueue,
      CommandListType aDstQueue) override;

    void ApplyViewportAndClipRect();
    void ApplyRenderTargets();
    void ApplyGraphicsPipelineState();
    void ApplyComputePipelineState();
    void ApplyResourceState();

    void ClearResourceState();

    VkDescriptorSet CreateDescriptorSet(VkDescriptorSetLayout aLayout);

    void BeginCommandBuffer();

    static std::unordered_map<uint64, VkPipeline> ourPipelineCache;
    static std::unordered_map<uint64, VkRenderPass> ourRenderpassCache;
    static std::unordered_map<uint64, VkFramebuffer> ourFramebufferCache;

    VkCommandBuffer myCommandBuffer;
    VkRenderPass myRenderPass;
    VkFramebuffer myFramebuffer;
    glm::uvec2 myFramebufferRes;

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
      VkImageSubresourceRange mySubresourceRange;
      VkImage myImage = nullptr;
      VkAccessFlags mySrcAccessMask = 0;
      VkAccessFlags myDstAccessMask = 0;
      VkImageLayout mySrcLayout = VK_IMAGE_LAYOUT_GENERAL;
      VkImageLayout myDstLayout = VK_IMAGE_LAYOUT_GENERAL;
      uint mySrcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      uint myDstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    };

    struct ResourceState
    {
      struct Descriptor
      {
        uint myBindingInSet = UINT_MAX;
        uint myFirstArrayElement = UINT_MAX;
        uint myNumDescriptors = UINT_MAX;
        VkDescriptorType myType = VK_DESCRIPTOR_TYPE_MAX_ENUM;
        VkDescriptorImageInfo* myImageInfos = nullptr;
        VkDescriptorBufferInfo* myBufferInfos = nullptr;
        VkBufferView* myTexelBufferView = nullptr;
      };

      struct DescriptorSet
      {
        uint mySet;
        VkDescriptorSetLayout myLayout;
        StaticArray<Descriptor, 64> myDescriptors;
      };

      StaticArray<DescriptorSet, 16> myDescriptorSets;
      StaticArray<VkDescriptorImageInfo, 128> myBoundImages;
      StaticArray<VkDescriptorBufferInfo, 128> myBoundBuffers;
      StaticArray<VkBufferView, 128> myBoundTexelBufferView;
    };

    ResourceState myResourceState;

    StaticArray<VkDescriptorPool, 16> myUsedDescriptorPools;
    BufferMemoryBarrierData myPendingBufferBarriers[kNumCachedBarriers];
    ImageMemoryBarrierData myPendingImageBarriers[kNumCachedBarriers];
    uint myNumPendingBufferBarriers;
    uint myNumPendingImageBarriers;

    VkPipelineStageFlags myPendingBarrierSrcStageMask;
    VkPipelineStageFlags myPendingBarrierDstStageMask;
  };
}



