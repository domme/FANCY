#pragma once

#include "CommandContext.h"
#include "DX12Prerequisites.h"
#include "DescriptorDX12.h"
#include <unordered_map>
#include "GpuResourceStorageDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class DescriptorDX12;
  class DescriptorHeapDX12;
  class GpuResourceDX12;
  class GpuResource;
  class GpuResourceStorageDX12;
//---------------------------------------------------------------------------//
  class CommandContextDX12 final : public CommandContext
  {
    friend class RenderCore_PlatformDX12;
    friend class CommandQueueDX12;

  public:
    static D3D12_DESCRIPTOR_HEAP_TYPE ResolveDescriptorHeapTypeFromMask(uint aDescriptorTypeMask);
    static D3D12_GRAPHICS_PIPELINE_STATE_DESC GetNativePSOdesc(const GraphicsPipelineState& aState);
    static D3D12_COMPUTE_PIPELINE_STATE_DESC GetNativePSOdesc(const ComputePipelineState& aState);

    CommandContextDX12(CommandListType aType);
    ~CommandContextDX12() override;

    void UpdateSubresources(ID3D12Resource* aDestResource, ID3D12Resource* aStagingResource, uint aFirstSubresourceIndex, uint aNumSubresources, D3D12_SUBRESOURCE_DATA* someSubresourceDatas) const;
    
    void ClearRenderTarget(TextureView* aTextureView, const float* aColor) override;
    void ClearDepthStencilTarget(TextureView* aTextureView, float aDepthClear, uint8 aStencilClear, uint someClearFlags = (uint)DepthStencilClearFlags::CLEAR_ALL) override;
    void CopyResource(GpuResource* aDestResource, GpuResource* aSrcResource) override;
    void CopyBufferRegion(const GpuBuffer* aDestBuffer, uint64 aDestOffset, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset, uint64 aSize) override;
    void CopyTextureRegion(const Texture* aDestTexture, const TextureSubLocation& aDestSubLocation, glm::uvec3 aDestTexelPos, const Texture* aSrcTexture, const TextureSubLocation& aSrcSubLocation, const TextureRegion* aSrcRegion = nullptr) override;
    void CopyTextureRegion(const Texture* aDestTexture, const TextureSubLocation& aDestSubLocation, const TextureRegion& aDestRegion, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset) override;

    void TransitionResourceList(const GpuResource** someResources, GpuResourceTransition* someTransitions, uint aNumResources) override;
    void Reset(uint64 aFenceVal) override;
    void SetGpuProgramPipeline(const SharedPtr<GpuProgramPipeline>& aGpuProgramPipeline) override;
    void BindVertexBuffer(const GpuBuffer* aBuffer, uint aVertexSize, uint64 anOffset = 0u, uint64 aSize = ~0ULL) override;
    void BindIndexBuffer(const GpuBuffer* aBuffer, uint anIndexSize, uint64 anOffset = 0u, uint64 aSize = ~0ULL) override;
    void Render(uint aNumIndicesPerInstance, uint aNumInstances, uint aStartIndex, uint aBaseVertex, uint aStartInstance) override;
    void RenderGeometry(const GeometryData* pGeometry) override;
    void BindBuffer(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someViewProperties, uint aRegisterIndex) const override;
    void BindResourceSet(const GpuResourceView** someResourceViews, uint aResourceCount, uint aRegisterIndex) override;

    void SetComputeProgram(const GpuProgram* aProgram) override;
    void Dispatch(uint GroupCountX, uint GroupCountY, uint GroupCountZ) override;

  protected:
    void CloseCommandList();
    void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType, DescriptorHeapDX12* aDescriptorHeap);
    void ApplyDescriptorHeaps();
    void ReleaseAllocator(uint64 aFenceVal);
    void ReleaseDynamicHeaps(uint64 aFenceVal);
    void ApplyViewportAndClipRect();
    void ApplyPipelineState();
    void ApplyGraphicsPipelineState();
    void ApplyComputePipelineState();
    void ApplyRenderTargets();
    void ApplyTopologyType();

    void SetResourceTransitionBarrier(const GpuResource* aResource, D3D12_RESOURCE_STATES aNewState) const;
    void SetResourceTransitionBarriers(const GpuResource** someResources, D3D12_RESOURCE_STATES* someNewStates, uint aNumResources) const;
    DescriptorDX12 CopyDescriptorsToDynamicHeapRange(const DescriptorDX12* someResources, uint aResourceCount);

    static std::unordered_map<uint64, ID3D12PipelineState*> ourPSOcache;
  
    ID3D12RootSignature* myRootSignature;  // The rootSignature that is set on myCommandList
    ID3D12GraphicsCommandList* myCommandList;
    ID3D12CommandAllocator* myCommandAllocator;
    bool myCommandListIsClosed;

    DescriptorHeapDX12* myDynamicShaderVisibleHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    std::vector<DescriptorHeapDX12*> myRetiredDescriptorHeaps; // TODO: replace vector with a smallObjectPool
  };
//---------------------------------------------------------------------------//
}
