#pragma once

#include "CommandContext.h"
#include "DX12Prerequisites.h"
#include "DescriptorDX12.h"
#include <unordered_map>

namespace Fancy {
//---------------------------------------------------------------------------//
  class DescriptorDX12;
  class DescriptorHeapDX12;
  class GpuResourceDX12;
  class GpuResource;
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
    void CopyTextureRegion(const Texture* aDestTexture, const TextureSubLocation& aDestSubLocation, const TextureRegion& aDestRegion, const Texture* aSrcTexture, const TextureSubLocation& aSrcSubLocation, const TextureRegion& aSrcRegion) override;
    void CopyTextureRegion(const Texture* aDestTexture, const TextureSubLocation& aDestSubLocation, const TextureRegion& aDestRegion, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset) override;

    void TransitionResourceList(GpuResource** someResources, GpuResourceState* someTransitionToStates, uint aNumResources) override;
    void Reset(uint64 aFenceVal) override;
    void SetGpuProgramPipeline(const SharedPtr<GpuProgramPipeline>& aGpuProgramPipeline) override;
    void SetVertexIndexBuffers(const GpuBuffer* aVertexBuffer, const GpuBuffer* anIndexBuffer, uint64 aVertexOffset = 0u, uint64 aNumVertices = ~0ULL, uint64 anIndexOffset = 0ULL, uint64 aNumIndices = ~0ULL) override;
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
