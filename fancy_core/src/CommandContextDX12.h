#pragma once

#include "FixedArray.h"
#include "GpuDynamicAllocatorDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {
  class DescriptorHeapDX12;
  

//---------------------------------------------------------------------------//
  class CommandContextDX12
  {
  public:
    CommandContextDX12(CommandListType aType);
    virtual ~CommandContextDX12();

    void Destroy();
    void Reset();

    static void InitBufferData(GpuBufferDX12* aBuffer, void* aDataPtr);
    static void UpdateBufferData(GpuBufferDX12* aBuffer, void* aDataPtr, uint32 aByteOffset, uint32 aByteSize);
    static void InitTextureData(TextureDX12* aTexture, const TextureUploadData* someUploadDatas, uint32 aNumUploadDatas);

    void ClearRenderTarget(Texture* aTexture, const float* aColor);
    void ClearDepthStencilTarget(Texture* aTexture, float aDepthClear, uint8 aStencilClear, uint32 someClearFlags = (uint32)DepthStencilClearFlags::CLEAR_ALL);

    // DX12-Specific stuff - TODO: Check if we need to find platform-independent ways to express these
    void UpdateSubresources(ID3D12Resource* aDestResource, ID3D12Resource* aStagingResource,
      uint32 aFirstSubresourceIndex, uint32 aNumSubresources, D3D12_SUBRESOURCE_DATA* someSubresourceDatas);
    void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType, DescriptorHeapDX12* aDescriptorHeap);
    void TransitionResource(GpuResourceDX12* aResource, D3D12_RESOURCE_STATES aDestState, bool aExecuteNow = false);
    //void CopySubresources(ID3D12Resource* aDestResource, ID3D12Resource* aSrcResource, uint aFirstSubresource, uint aSubResourceCount);

    void CopyResource(GpuResourceDX12* aDestResource, GpuResourceDX12* aSrcResource);

    uint64 ExecuteAndReset(bool aWaitForCompletion = false);

    CommandListType GetType() const { return myCommandListType; }

  protected:
    virtual void ResetInternal();

    void ApplyDescriptorHeaps();
    void KickoffResourceBarriers();
    void ReleaseAllocator(uint64 aFenceVal);
    void ReleaseDynamicHeaps(uint64 aFenceVal);

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