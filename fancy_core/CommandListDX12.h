#pragma once

#include "CommandList.h"
#include "DX12Prerequisites.h"
#include "DescriptorDX12.h"

#include "GpuResourceDataDX12.h"
#include "RenderEnums.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class DescriptorDX12;
  class DynamicDescriptorHeapDX12;
  class GpuResourceDX12;
  class GpuResource;
  struct GpuResourceDataDX12;
  class GpuProgram;
  class GpuBuffer;
//---------------------------------------------------------------------------//
  class CommandListDX12 final : public CommandList
  {
    friend class RenderCore_PlatformDX12;
    friend class CommandQueueDX12;

  public:
    static D3D12_DESCRIPTOR_HEAP_TYPE ResolveDescriptorHeapTypeFromMask(uint aDescriptorTypeMask);
    static D3D12_GRAPHICS_PIPELINE_STATE_DESC GetNativePSOdesc(const GraphicsPipelineState& aState);
    static D3D12_COMPUTE_PIPELINE_STATE_DESC GetNativePSOdesc(const ComputePipelineState& aState);

    CommandListDX12(CommandListType aType, uint someFlags);
    ~CommandListDX12() override;

    void UpdateSubresources(ID3D12Resource* aDestResource, ID3D12Resource* aStagingResource, uint aFirstSubresourceIndex, uint aNumSubresources, D3D12_SUBRESOURCE_DATA* someSubresourceDatas);
    
    void ClearRenderTarget(TextureView* aTextureView, const float* aColor) override;
    void ClearDepthStencilTarget(TextureView* aTextureView, float aDepthClear, uint8 aStencilClear, uint someClearFlags = (uint)DepthStencilClearFlags::CLEAR_ALL) override;
    void CopyResource(GpuResource* aDestResource, GpuResource* aSrcResource) override;
    void CopyBufferRegion(const GpuBuffer* aDestBuffer, uint64 aDestOffset, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset, uint64 aSize) override;
    void CopyTextureRegion(const GpuBuffer* aDestBuffer, uint64 aDestOffset, const Texture* aSrcTexture, const TextureSubLocation& aSrcSubLocation, const TextureRegion* aSrcRegion = nullptr) override;
    void CopyTextureRegion(const Texture* aDestTexture, const TextureSubLocation& aDestSubLocation, const glm::uvec3& aDestTexelPos, const Texture* aSrcTexture, const TextureSubLocation& aSrcSubLocation, const TextureRegion* aSrcRegion = nullptr) override;
    void CopyTextureRegion(const Texture* aDestTexture, const TextureSubLocation& aDestSubLocation, const glm::uvec3& aDestTexelPos, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset) override;

    void ReleaseGpuResources(uint64 aFenceVal) override;
    void Reset(uint someFlags) override;
    void FlushBarriers() override;
    void SetGpuProgramPipeline(const SharedPtr<GpuProgramPipeline>& aGpuProgramPipeline) override;
    void BindVertexBuffer(const GpuBuffer* aBuffer, uint aVertexSize, uint64 anOffset = 0u, uint64 aSize = ~0ULL) override;
    void BindIndexBuffer(const GpuBuffer* aBuffer, uint anIndexSize, uint64 anOffset = 0u, uint64 aSize = ~0ULL) override;
    void Render(uint aNumIndicesPerInstance, uint aNumInstances, uint aStartIndex, uint aBaseVertex, uint aStartInstance) override;
    void RenderGeometry(const GeometryData* pGeometry) override;
    void BindBuffer(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someViewProperties, uint aRegisterIndex) const override;
    void BindResourceSet(const GpuResourceView** someResourceViews, uint aResourceCount, uint aRegisterIndex) override;

    GpuQuery BeginQuery(GpuQueryType aType) override;
    void EndQuery(const GpuQuery& aQuery) override;
    GpuQuery InsertTimestamp() override;
    void CopyQueryDataToBuffer(const GpuQueryHeap* aQueryHeap, const GpuBuffer* aBuffer, uint aFirstQueryIndex, uint aNumQueries, uint64 aBufferOffset) override;

    void ResourceUAVbarrier(const GpuResource** someResources = nullptr, uint aNumResources = 0u) override;

    void SubresourceBarrierInternal(
      const GpuResource** someResources,
      const uint16** someSubResourceLists,
      const uint* someNumSubresources,
      const GpuResourceUsageState* someSrcStates,
      const GpuResourceUsageState* someDstStates,
      uint aNumResources,
      CommandListType aSrcQueue,
      CommandListType aDstQueue) override;

    bool IsOpen() const override { return myIsOpen; }
    void Close() override;

    void SetComputeProgram(const GpuProgram* aProgram) override;
    void Dispatch(const glm::int3& aNumThreads) override;

  protected:
    void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType, DynamicDescriptorHeapDX12* aDescriptorHeap);
    void ApplyDescriptorHeaps();
    void ApplyViewportAndClipRect();
    void ApplyGraphicsPipelineState();
    void ApplyComputePipelineState();
    void ApplyRenderTargets();
    void ApplyTopologyType();

#if FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES
    void SetTrackResourceTransitionBarrier(const GpuResource* aResource, D3D12_RESOURCE_STATES aNewState) const;
    void SetTrackResourceTransitionBarriers(const GpuResource** someResources, const D3D12_RESOURCE_STATES* someNewStates, uint aNumStates) const;
    void SetTrackSubresourceTransitionBarrier(const GpuResource* aResource, uint16 aSubresourceIndex, D3D12_RESOURCE_STATES aNewState) const;
    void SetTrackSubresourceTransitionBarriers(const GpuResource** someResources, const D3D12_RESOURCE_STATES* someNewStates, const uint16** someSubresourceLists, const uint* someNumSubresources, uint aNumStates) const;
#endif 

    DescriptorDX12 CopyDescriptorsToDynamicHeapRange(const DescriptorDX12* someResources, uint aResourceCount);

    static std::unordered_map<uint64, ID3D12PipelineState*> ourPSOcache;
  
    bool myIsOpen;
    ID3D12RootSignature* myRootSignature;  // The rootSignature that is set on myCommandList
    ID3D12RootSignature* myComputeRootSignature;
    ID3D12GraphicsCommandList* myCommandList;
    ID3D12CommandAllocator* myCommandAllocator;
    D3D12_RESOURCE_BARRIER myPendingBarriers[256];
    uint myNumPendingBarriers;

    DynamicDescriptorHeapDX12* myDynamicShaderVisibleHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    std::vector<DynamicDescriptorHeapDX12*> myRetiredDescriptorHeaps; // TODO: replace vector with a smallObjectPool
  };
//---------------------------------------------------------------------------//
}
