#pragma once

#include "FixedArray.h"
#include "CommandContext.h"
#include "GpuDynamicAllocatorDX12.h"
#include "DX12Prerequisites.h"
#include "DescriptorDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class DescriptorDX12;
  class DescriptorHeapDX12;
//---------------------------------------------------------------------------//
  class DLLEXPORT CommandContextDX12
  {
    friend class RenderCore_PlatformDX12;

  public:
    CommandContextDX12(CommandListType aType);
    virtual ~CommandContextDX12();

    static D3D12_DESCRIPTOR_HEAP_TYPE ResolveDescriptorHeapTypeFromMask(uint32 aDescriptorTypeMask);

    void Destroy();

    void UpdateSubresources(ID3D12Resource* aDestResource, ID3D12Resource* aStagingResource, uint32 aFirstSubresourceIndex, uint32 aNumSubresources, D3D12_SUBRESOURCE_DATA* someSubresourceDatas) const;
    void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType, DescriptorHeapDX12* aDescriptorHeap);
    void TransitionResource(GpuResourceDX12* aResource, D3D12_RESOURCE_STATES aDestState, bool aExecuteNow = false);
    void CopyResource(GpuResourceDX12* aDestResource, GpuResourceDX12* aSrcResource);

  protected:
    virtual void Reset_Internal();
    virtual uint64 ExecuteAndReset_Internal(bool aWaitForCompletion = false);

    void ResetRootSignatureAndHeaps();
    void ApplyDescriptorHeaps();
    void KickoffResourceBarriers();
    void ReleaseAllocator(uint64 aFenceVal);
    void ReleaseDynamicHeaps(uint64 aFenceVal);

    void ClearRenderTarget_Internal(Texture* aTexture, const float* aColor);
    void ClearDepthStencilTarget_Internal(Texture* aTexture, float aDepthClear, uint8 aStencilClear, uint32 someClearFlags = (uint32)DepthStencilClearFlags::CLEAR_ALL);

    static const GpuResourceDX12* CastGpuResourceDX12(const GpuResource* aResource);
    
    DescriptorDX12 CopyDescriptorsToDynamicHeapRange(const DescriptorDX12** someResources, uint32 aResourceCount);
  
    CommandListType myCommandListType;
    ID3D12RootSignature* myRootSignature;  // The rootSignature that is set on myCommandList
    ID3D12GraphicsCommandList* myCommandList;

    ID3D12CommandAllocator* myCommandAllocator;
    FixedArray<D3D12_RESOURCE_BARRIER, 16u> myWaitingResourceBarriers;

    DescriptorHeapDX12* myDynamicShaderVisibleHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    std::vector<DescriptorHeapDX12*> myRetiredDescriptorHeaps; // TODO: replace vector with a smallObjectPool

    bool myIsInRecordState;

    GpuDynamicAllocatorDX12 myCpuVisibleAllocator;
    GpuDynamicAllocatorDX12 myGpuOnlyAllocator;
  };
//---------------------------------------------------------------------------//
} } }
