#pragma once
#include "CommandList.h"
#include "MathIncludes.h"
#include "VkPrerequisites.h"
#include <glm/detail/type_mat.hpp>

namespace Fancy
{
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
    void RenderGeometry(const GeometryData* pGeometry) override;
    void UpdateTextureData(const Texture* aDstTexture, const SubresourceRange& aSubresourceRange, const TextureSubData* someDatas, uint aNumDatas /*, const TextureRegion* someRegions = nullptr */) override;

    void BindResourceView(const GpuResourceView* aView, const char* aName) override;
    void BindBuffer(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someViewProperties, const char* aName) override;
    
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

    BufferMemoryBarrierData myPendingBufferBarriers[kNumCachedBarriers];
    ImageMemoryBarrierData myPendingImageBarriers[kNumCachedBarriers];
    uint myNumPendingBufferBarriers;
    uint myNumPendingImageBarriers;

    VkPipelineStageFlags myPendingBarrierSrcStageMask;
    VkPipelineStageFlags myPendingBarrierDstStageMask;
  };
}



