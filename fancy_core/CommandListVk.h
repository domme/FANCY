#pragma once
#include "CommandList.h"
#include "MathIncludes.h"
#include "VkPrerequisites.h"

namespace Fancy
{
  class CommandListVk : public CommandList
  {
  public:
    CommandListVk(CommandListType aType);
    ~CommandListVk();

    void ClearRenderTarget(TextureView* aTextureView, const float* aColor) override;
    void ClearDepthStencilTarget(TextureView* aTextureView, float aDepthClear, uint8 aStencilClear, uint someClearFlags) override;
    void CopyResource(GpuResource* aDestResource, GpuResource* aSrcResource) override;
    void CopyBufferRegion(const GpuBuffer* aDestBuffer, uint64 aDestOffset, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset, uint64 aSize) override;
    void CopyTextureRegion(const GpuBuffer* aDestBuffer, uint64 aDestOffset, const Texture* aSrcTexture, const TextureSubLocation& aSrcSubLocation, const TextureRegion* aSrcRegion) override;
    void CopyTextureRegion(const Texture* aDestTexture, const TextureSubLocation& aDestSubLocation, const glm::uvec3& aDestTexelPos, const Texture* aSrcTexture, const TextureSubLocation& aSrcSubLocation, const TextureRegion* aSrcRegion) override;
    void CopyTextureRegion(const Texture* aDestTexture, const TextureSubLocation& aDestSubLocation, const glm::uvec3& aDestTexelPos, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset) override;

    void ReleaseGpuResources(uint64 aFenceVal) override;
    void Reset() override;
    void FlushBarriers() override;
    void SetShaderPipeline(const SharedPtr<ShaderPipeline>& aShaderPipeline) override;
    void BindVertexBuffer(const GpuBuffer* aBuffer, uint aVertexSize, uint64 anOffset, uint64 aSize) override;
    void BindIndexBuffer(const GpuBuffer* aBuffer, uint anIndexSize, uint64 anOffset, uint64 aSize) override;
    void Render(uint aNumIndicesPerInstance, uint aNumInstances, uint aStartIndex, uint aBaseVertex, uint aStartInstance) override;
    void RenderGeometry(const GeometryData* pGeometry) override;
    void BindBuffer(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someViewProperties, uint aRegisterIndex) const override;
    void BindResourceSet(const GpuResourceView** someResourceViews, uint aResourceCount, uint aRegisterIndex) override;
    
    GpuQuery BeginQuery(GpuQueryType aType) override;
    void EndQuery(const GpuQuery& aQuery) override;
    GpuQuery InsertTimestamp() override;
    void CopyQueryDataToBuffer(const GpuQueryHeap* aQueryHeap, const GpuBuffer* aBuffer, uint aFirstQueryIndex, uint aNumQueries, uint64 aBufferOffset) override;

    void ResourceUAVbarrier(const GpuResource** someResources, uint aNumResources) override;

    void Close() override;
    bool IsOpen() const override;
    
    void SetComputeProgram(const Shader* aProgram) override;
    void Dispatch(const glm::int3& aNumThreads) override;
    
  protected:
    bool SubresourceBarrierInternal(
      const GpuResource* aResource,
      const uint16* someSubresources,
      uint aNumSubresources,
      GpuResourceState aSrcState,
      GpuResourceState aDstState,
      CommandListType aSrcQueue,
      CommandListType aDstQueue) override;

    void ApplyViewportAndClipRect();
    void ApplyRenderTargets();
    void ApplyTopologyType();
    void ApplyGraphicsPipelineState();
    void ApplyComputePipelineState();

    static VkPipeline CreateGraphicsPipeline(const GraphicsPipelineState& aState, VkRenderPass aRenderPass);
    static VkComputePipelineCreateInfo GetComputePipelineCreateInfo(const ComputePipelineState& aState);

    static std::unordered_map<uint64, VkPipeline> ourPipelineCache;

    bool myIsOpen;
    VkCommandBuffer myCommandBuffer;
  };
}



