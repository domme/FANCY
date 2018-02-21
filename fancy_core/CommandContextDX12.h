#pragma once

#include "CommandContext.h"
#include "DX12Prerequisites.h"
#include "DescriptorDX12.h"
#include <unordered_map>

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class DescriptorDX12;
  class DescriptorHeapDX12;
  class GpuResourceDX12;
//---------------------------------------------------------------------------//
  class CommandContextDX12 : public CommandContext
  {
    friend class RenderCore_PlatformDX12;

  public:
    static D3D12_DESCRIPTOR_HEAP_TYPE ResolveDescriptorHeapTypeFromMask(uint aDescriptorTypeMask);
    static D3D12_GRAPHICS_PIPELINE_STATE_DESC GetNativePSOdesc(const GraphicsPipelineState& aState);
    static D3D12_COMPUTE_PIPELINE_STATE_DESC GetNativePSOdesc(const ComputePipelineState& aState);

    CommandContextDX12(CommandListType aType);
    ~CommandContextDX12() override;

    void UpdateSubresources(ID3D12Resource* aDestResource, ID3D12Resource* aStagingResource, uint aFirstSubresourceIndex, uint aNumSubresources, D3D12_SUBRESOURCE_DATA* someSubresourceDatas) const;
    
    void ClearRenderTarget(Texture* aTexture, const float* aColor) override;
    void ClearDepthStencilTarget(Texture* aTexture, float aDepthClear, uint8 aStencilClear, uint someClearFlags = (uint)DepthStencilClearFlags::CLEAR_ALL) override;
    void CopyResource(GpuResource* aDestResource, GpuResource* aSrcResource) override;
    void TransitionResourceList(GpuResource** someResources, GpuResourceState* someTransitionToStates, uint aNumResources) override;
    uint64 ExecuteAndReset(bool aWaitForCompletion) override;
    void Reset() override;
    void SetGpuProgramPipeline(const SharedPtr<GpuProgramPipeline>& aGpuProgramPipeline) override;
    void SetVertexIndexBuffers(const Rendering::GpuBuffer* aVertexBuffer, const Rendering::GpuBuffer* anIndexBuffer, uint aVertexOffset = 0u, uint aNumVertices = UINT_MAX, uint anIndexOffset = 0u, uint aNumIndices = UINT_MAX) override;
    void Render(uint aNumIndicesPerInstance, uint aNumInstances, uint anIndexOffset, uint aVertexOffset, uint anInstanceOffset) override;
    void RenderGeometry(const Geometry::GeometryData* pGeometry) override;
    void BindResource(const GpuResource* aResource, DescriptorType aBindingType, uint aRegisterIndex) const override;
    void BindDescriptorSet(const Descriptor** someDescriptors, uint aResourceCount, uint aRegisterIndex) override;

    void SetComputeProgram(const GpuProgram* aProgram) override;
    void Dispatch(uint GroupCountX, uint GroupCountY, uint GroupCountZ) override;

  protected:
    void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType, DescriptorHeapDX12* aDescriptorHeap);
    void ApplyDescriptorHeaps();
    void ReleaseAllocator(uint64 aFenceVal);
    void ReleaseDynamicHeaps(uint64 aFenceVal);
    void ApplyViewportAndClipRect();
    void ApplyPipelineState();
    void ApplyGraphicsPipelineState();
    void ApplyComputePipelineState();
    void ApplyRenderTargets();
    
    DescriptorDX12 CopyDescriptorsToDynamicHeapRange(const DescriptorDX12** someResources, uint aResourceCount);

    static std::unordered_map<uint64, ID3D12PipelineState*> ourPSOcache;
  
    ID3D12RootSignature* myRootSignature;  // The rootSignature that is set on myCommandList
    ID3D12GraphicsCommandList* myCommandList;
    ID3D12CommandAllocator* myCommandAllocator;

    DescriptorHeapDX12* myDynamicShaderVisibleHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    std::vector<DescriptorHeapDX12*> myRetiredDescriptorHeaps; // TODO: replace vector with a smallObjectPool
  };
//---------------------------------------------------------------------------//
} } }
