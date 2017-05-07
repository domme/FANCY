#include "FancyCorePrerequisites.h"
#include "CommandContextBaseDX12.h"
#include "GpuBufferDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"
#include "TextureDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  namespace {
  //---------------------------------------------------------------------------//
    D3D12_COMMAND_LIST_TYPE locResolveCommandListType(CommandListType aCommandListType)
    {
      switch(aCommandListType)
      {
        case CommandListType::Graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
        case CommandListType::Compute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
        case CommandListType::DMA: return D3D12_COMMAND_LIST_TYPE_COPY;
        default:
          ASSERT(false, "CommandListType % not implemented", (uint32)aCommandListType);
          return D3D12_COMMAND_LIST_TYPE_DIRECT;
      }
    }
  //---------------------------------------------------------------------------//
  }
  
//---------------------------------------------------------------------------//
  CommandContextBaseDX12::CommandContextBaseDX12(CommandListType aCommandListType)
    : myCommandListType(aCommandListType)
    , myCpuVisibleAllocator(aCommandListType, GpuDynamicAllocatorType::CpuWritable)
    , myGpuOnlyAllocator(aCommandListType, GpuDynamicAllocatorType::GpuOnly)
    , myRootSignature(nullptr)
    , myCommandList(nullptr)
    , myCommandAllocator(nullptr)
    , myIsInRecordState(true)
  {
    memset(myDynamicShaderVisibleHeaps, 0u, sizeof(myDynamicShaderVisibleHeaps));

    myCommandAllocator = RenderCore::GetPlatformDX12()->GetCommandAllocator(myCommandListType);

    D3D12_COMMAND_LIST_TYPE nativeCmdListType = locResolveCommandListType(aCommandListType);

    CheckD3Dcall(
      RenderCore::GetPlatformDX12()->GetDevice()->CreateCommandList(0, nativeCmdListType,
        myCommandAllocator, nullptr, IID_PPV_ARGS(&myCommandList))
      );
  }
//---------------------------------------------------------------------------//
  CommandContextBaseDX12::~CommandContextBaseDX12()
  {
    Destroy();
  }
//---------------------------------------------------------------------------//
  D3D12_DESCRIPTOR_HEAP_TYPE CommandContextBaseDX12::ResolveDescriptorHeapTypeFromMask(uint32 aDescriptorTypeMask)
  {
    if (aDescriptorTypeMask & (uint32)GpuDescriptorTypeFlags::BUFFER_TEXTURE_CONSTANT_BUFFER)
      return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    else if (aDescriptorTypeMask & (uint32)GpuDescriptorTypeFlags::SAMPLER)
      return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;

    ASSERT(false, "unsupported descriptor type mask");
    return (D3D12_DESCRIPTOR_HEAP_TYPE)-1;
  }
//---------------------------------------------------------------------------//
  void CommandContextBaseDX12::Destroy()
  {
    if (myCommandList != nullptr)
      myCommandList->Release();

    myCommandList = nullptr;

    if (myCommandAllocator != nullptr)
      ReleaseAllocator(0u);
  }
//---------------------------------------------------------------------------//
  void CommandContextBaseDX12::Reset_Internal()
  {
    if (myIsInRecordState)
      return;

    ASSERT(nullptr == myCommandAllocator, "myIsInRecordState-flag out of sync");

    myCommandAllocator = RenderCore::GetPlatformDX12()->GetCommandAllocator(myCommandListType);
    ASSERT(myCommandAllocator != nullptr);

    CheckD3Dcall(myCommandList->Reset(myCommandAllocator, nullptr));

    myIsInRecordState = true;

    ResetRootSignatureAndHeaps();
  }
//---------------------------------------------------------------------------//
  uint64 CommandContextBaseDX12::ExecuteAndReset_Internal(bool aWaitForCompletion)
  {
    KickoffResourceBarriers();

    ASSERT(myCommandAllocator != nullptr && myCommandList != nullptr);
    CheckD3Dcall(myCommandList->Close());
    myIsInRecordState = false;

    uint64 fenceVal = RenderCore::GetPlatformDX12()->ExecuteCommandList(myCommandList);

    myCpuVisibleAllocator.CleanupAfterCmdListExecute(fenceVal);
    myGpuOnlyAllocator.CleanupAfterCmdListExecute(fenceVal);
    ReleaseDynamicHeaps(fenceVal);
    ReleaseAllocator(fenceVal);

    if (aWaitForCompletion)
      RenderCore::GetPlatformDX12()->WaitForFence(myCommandListType, fenceVal);

    Reset_Internal();

    return fenceVal;
  }
//---------------------------------------------------------------------------//
  void CommandContextBaseDX12::ResetRootSignatureAndHeaps()
  {
    myRootSignature = nullptr;
    memset(myDynamicShaderVisibleHeaps, 0u, sizeof(myDynamicShaderVisibleHeaps));
  }
//---------------------------------------------------------------------------//
  void CommandContextBaseDX12::SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType, DescriptorHeapDX12* aDescriptorHeap)
  {
    if (myDynamicShaderVisibleHeaps[aHeapType] == aDescriptorHeap)
      return;

    // Has this heap already been used on this commandList? Then we need to "remember" it until ExecuteAndReset()
    if (myDynamicShaderVisibleHeaps[aHeapType] != nullptr)
      myRetiredDescriptorHeaps.push_back(myDynamicShaderVisibleHeaps[aHeapType]);

    myDynamicShaderVisibleHeaps[aHeapType] = aDescriptorHeap;
    ApplyDescriptorHeaps();
  }
  //---------------------------------------------------------------------------//
  void CommandContextBaseDX12::ClearRenderTarget(Texture* aTexture, const float* aColor)
  {
    ASSERT(aTexture->GetParameters().myIsRenderTarget);

    TransitionResource(static_cast<TextureDX12*>(aTexture), D3D12_RESOURCE_STATE_RENDER_TARGET, true);
    myCommandList->ClearRenderTargetView(static_cast<TextureDX12*>(aTexture)->GetRtv().myCpuHandle, aColor, 0, nullptr);
  }
  //---------------------------------------------------------------------------//
  void CommandContextBaseDX12::ClearDepthStencilTarget(Texture* aTexture, float aDepthClear,
    uint8 aStencilClear, uint32 someClearFlags /* = (uint32)DepthStencilClearFlags::CLEAR_ALL */)
  {
    ASSERT(aTexture->GetParameters().bIsDepthStencil);

    D3D12_CLEAR_FLAGS clearFlags = (D3D12_CLEAR_FLAGS)0;
    if (someClearFlags & (uint32)DepthStencilClearFlags::CLEAR_DEPTH)
      clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
    if (someClearFlags & (uint32)DepthStencilClearFlags::CLEAR_STENCIL)
      clearFlags |= D3D12_CLEAR_FLAG_STENCIL;

    myCommandList->ClearDepthStencilView(static_cast<TextureDX12*>(aTexture)->GetDsv().myCpuHandle, clearFlags, aDepthClear, aStencilClear, 0, nullptr);
  }
  //---------------------------------------------------------------------------//
  void CommandContextBaseDX12::TransitionResource(GpuResourceDX12* aResource, D3D12_RESOURCE_STATES aDestState, bool aExecuteNow /* = false */)
  {
    if (aResource->GetUsageState() == aDestState)
      return;

    D3D12_RESOURCE_BARRIER barrier;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = aResource->GetResource();
    barrier.Transition.StateBefore = aResource->GetUsageState();
    barrier.Transition.StateAfter = aDestState;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    aResource->myUsageState = aDestState;

    myWaitingResourceBarriers.push_back(barrier);

    if (aExecuteNow || myWaitingResourceBarriers.IsFull())
    {
      KickoffResourceBarriers();
    }
  }
//---------------------------------------------------------------------------//
  static void locMemcpySubresourceRows(const D3D12_MEMCPY_DEST* aDest, const D3D12_SUBRESOURCE_DATA* aSrc, size_t aRowStrideBytes, uint32 aNumRows, uint32 aNumSlices)
  {
    for (uint32 iSlice = 0u; iSlice < aNumSlices; ++iSlice)
    {
      uint8* destSliceDataPtr = static_cast<uint8*>(aDest->pData) + aDest->SlicePitch * iSlice;
      const uint8* srcSliceDataPtr = static_cast<const uint8*>(aSrc->pData) + aSrc->SlicePitch * iSlice;
      for (uint32 iRow = 0u; iRow < aNumRows; ++iRow)
      {
        uint8* destDataPtr = destSliceDataPtr + aDest->RowPitch * iRow;
        const uint8* srcDataPtr = srcSliceDataPtr + aSrc->RowPitch * iRow;

        memcpy(destDataPtr, srcDataPtr, aRowStrideBytes);
      }
    }
  }
//---------------------------------------------------------------------------//
  void CommandContextBaseDX12::UpdateSubresources(ID3D12Resource* aDestResource, ID3D12Resource* aStagingResource,
    uint32 aFirstSubresourceIndex, uint32 aNumSubresources, D3D12_SUBRESOURCE_DATA* someSubresourceDatas)
  {
    D3D12_RESOURCE_DESC srcDesc = aStagingResource->GetDesc();
    D3D12_RESOURCE_DESC destDesc = aDestResource->GetDesc();

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT* destLayouts = static_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(alloca(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) * aNumSubresources));
    uint64* destRowSizesByte = static_cast<uint64*>(alloca(sizeof(uint64) * aNumSubresources));
    uint32* destRowNums = static_cast<uint32*>(alloca(sizeof(uint32) * aNumSubresources));

    uint64 destTotalSizeBytes = 0u;
    RenderCore::GetPlatformDX12()->GetDevice()->GetCopyableFootprints(&destDesc, aFirstSubresourceIndex, aNumSubresources, 0u, destLayouts, destRowNums, destRowSizesByte, &destTotalSizeBytes);

    // Prepare a temporary buffer that contains all subresource data in the expected form (i.e. respecting the dest data layout)
    uint8* tempBufferDataPtr;
    if (S_OK != aStagingResource->Map(0, nullptr, reinterpret_cast<void**>(&tempBufferDataPtr)))
      return;

    for (uint32 i = 0u; i < aNumSubresources; ++i)
    {
      D3D12_MEMCPY_DEST dest;
      dest.pData = tempBufferDataPtr + destLayouts[i].Offset;
      dest.RowPitch = destLayouts[i].Footprint.RowPitch;
      dest.SlicePitch = destLayouts[i].Footprint.RowPitch * destRowNums[i];
      locMemcpySubresourceRows(&dest, &someSubresourceDatas[i], destRowSizesByte[i], destRowNums[i], destLayouts[i].Footprint.Depth);
    }
    aStagingResource->Unmap(0, nullptr);

    // Copy from the temp staging buffer to the destination resource (could be buffer or texture)
    if (destDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
    {
      myCommandList->CopyBufferRegion(aDestResource, 0, aStagingResource, destLayouts[0].Offset, destLayouts[0].Footprint.Width);
    }
    else
    {
      for (uint32 i = 0u; i < aNumSubresources; ++i)
      {
        D3D12_TEXTURE_COPY_LOCATION destCopyLocation;
        destCopyLocation.pResource = aDestResource;
        destCopyLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        destCopyLocation.SubresourceIndex = aFirstSubresourceIndex + i;

        D3D12_TEXTURE_COPY_LOCATION srcCopyLocation;
        srcCopyLocation.pResource = aStagingResource;
        srcCopyLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        srcCopyLocation.PlacedFootprint = destLayouts[i];
        myCommandList->CopyTextureRegion(&destCopyLocation, 0u, 0u, 0u, &srcCopyLocation, nullptr);
      }
    }
  }
  //---------------------------------------------------------------------------//
  void CommandContextBaseDX12::CopyResource(GpuResourceDX12* aDestResource, GpuResourceDX12* aSrcResource)
  {
    D3D12_RESOURCE_STATES oldDestState = aDestResource->GetUsageState();
    D3D12_RESOURCE_STATES oldSrcState = aSrcResource->GetUsageState();

    TransitionResource(aDestResource, D3D12_RESOURCE_STATE_COPY_DEST);
    TransitionResource(aSrcResource, D3D12_RESOURCE_STATE_COPY_SOURCE);
    KickoffResourceBarriers();

    myCommandList->CopyResource(aDestResource->GetResource(), aSrcResource->GetResource());

    TransitionResource(aDestResource, oldDestState);
    TransitionResource(aSrcResource, oldSrcState);
    KickoffResourceBarriers();
  }
//---------------------------------------------------------------------------//
  void CommandContextBaseDX12::InitBufferData(GpuBufferDX12* aBuffer, void* aDataPtr)
  {
    D3D12_HEAP_PROPERTIES heapProps;
    memset(&heapProps, 0, sizeof(heapProps));
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    const D3D12_RESOURCE_DESC& resourceDesc = aBuffer->GetResource()->GetDesc();

    ComPtr<ID3D12Resource> uploadResource;
    CheckD3Dcall(RenderCore::GetPlatformDX12()->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
      &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadResource)));

    void* mappedBufferPtr;
    CheckD3Dcall(uploadResource->Map(0, nullptr, &mappedBufferPtr));
    memcpy(mappedBufferPtr, aDataPtr, aBuffer->GetSizeBytes());
    uploadResource->Unmap(0, nullptr);

    CommandContext* initContext = CommandContextPool::AllocateContext(CommandListType::Graphics);
    initContext->TransitionResource(aBuffer, D3D12_RESOURCE_STATE_COPY_DEST, true);
    initContext->myCommandList->CopyResource(aBuffer->GetResource(), uploadResource.Get());
    initContext->TransitionResource(aBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, true);

    initContext->ExecuteAndReset(true);

    RenderContext::FreeContext(initContext);
  }
  //---------------------------------------------------------------------------//
  void CommandContextBaseDX12::UpdateBufferData(GpuBufferDX12* aBuffer, void* aDataPtr, uint32 aByteOffset, uint32 aByteSize)
  {
    D3D12_HEAP_PROPERTIES heapProps;
    memset(&heapProps, 0, sizeof(heapProps));
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC uploadResourceDesc = aBuffer->GetResource()->GetDesc();
    uploadResourceDesc.Width = MathUtil::Align(aByteSize, aBuffer->GetAlignment());

    ComPtr<ID3D12Resource> uploadResource;
    CheckD3Dcall(RenderCore::GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
      &uploadResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadResource)));

    void* uploadBufferPtr;
    CheckD3Dcall(uploadResource->Map(0, nullptr, &uploadBufferPtr));
    memcpy(uploadBufferPtr, aDataPtr, aByteSize);
    uploadResource->Unmap(0, nullptr);

    CommandContext* initContext = RenderContext::AllocateContext(CommandListType::Graphics);
    initContext->TransitionResource(aBuffer, D3D12_RESOURCE_STATE_COPY_DEST, true);
    initContext->myCommandList->CopyBufferRegion(aBuffer->GetResource(), aByteOffset, uploadResource.Get(), 0u, aByteSize);
    initContext->TransitionResource(aBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, true);

    initContext->ExecuteAndReset(true);

    RenderContext::FreeContext(initContext);
  }
  //---------------------------------------------------------------------------//
  void CommandContextBaseDX12::InitTextureData(TextureDX12* aTexture, const TextureUploadData* someUploadDatas, uint32 aNumUploadDatas)
  {
    ID3D12Device* device = RenderCore::GetDevice();
    const D3D12_RESOURCE_DESC& resourceDesc = aTexture->GetResource()->GetDesc();

    // DEBUG: layouts and row-infos not needed here yet
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT* destLayouts = static_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(alloca(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) * aNumUploadDatas));
    uint64* destRowSizesByte = static_cast<uint64*>(alloca(sizeof(uint64) * aNumUploadDatas));
    uint32* destRowNums = static_cast<uint32*>(alloca(sizeof(uint32) * aNumUploadDatas));

    uint64 requiredStagingBufferSize;
    //device->GetCopyableFootprints(&resourceDesc, 0u, aNumUploadDatas, 0u, nullptr, nullptr, nullptr, &requiredStagingBufferSize);
    device->GetCopyableFootprints(&resourceDesc, 0u, aNumUploadDatas, 0u, destLayouts, destRowNums, destRowSizesByte, &requiredStagingBufferSize);

    D3D12_HEAP_PROPERTIES HeapProps;
    HeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    HeapProps.CreationNodeMask = 1;
    HeapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC BufferDesc;
    BufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    BufferDesc.Alignment = 0;
    BufferDesc.Width = requiredStagingBufferSize;
    BufferDesc.Height = 1;
    BufferDesc.DepthOrArraySize = 1;
    BufferDesc.MipLevels = 1;
    BufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    BufferDesc.SampleDesc.Count = 1;
    BufferDesc.SampleDesc.Quality = 0;
    BufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    BufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ComPtr<ID3D12Resource> stagingBuffer;

    CheckD3Dcall(device->CreateCommittedResource(
      &HeapProps, D3D12_HEAP_FLAG_NONE,
      &BufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr, IID_PPV_ARGS(&stagingBuffer)));

    ASSERT(aTexture->getParameters().u16Depth <= 1u, "The code below might not work for 3D textures");

    D3D12_SUBRESOURCE_DATA* subDatas = static_cast<D3D12_SUBRESOURCE_DATA*>(alloca(sizeof(D3D12_SUBRESOURCE_DATA) * aNumUploadDatas));
    for (uint32 i = 0u; i < aNumUploadDatas; ++i)
    {
      subDatas[i].pData = someUploadDatas[i].myData;
      subDatas[i].SlicePitch = someUploadDatas[i].mySliceSizeBytes;
      subDatas[i].RowPitch = someUploadDatas[i].myRowSizeBytes;
    }

    CommandContext* uploadContext = CommandContext::AllocateContext(CommandListType::Graphics);
    D3D12_RESOURCE_STATES oldUsageState = aTexture->GetUsageState();
    uploadContext->TransitionResource(aTexture, D3D12_RESOURCE_STATE_COPY_DEST, true);
    uploadContext->UpdateSubresources(aTexture->GetResource(), stagingBuffer.Get(), 0u, aNumUploadDatas, subDatas);
    uploadContext->TransitionResource(aTexture, oldUsageState, true);

    uploadContext->ExecuteAndReset(true);

    RenderContext::FreeContext(uploadContext);
  }
//---------------------------------------------------------------------------//
  void CommandContextBaseDX12::ApplyDescriptorHeaps()
  {
    ID3D12DescriptorHeap* heapsToBind[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    memset(heapsToBind, 0, sizeof(heapsToBind));
    uint32 numHeapsToBind = 0u;

    for (DescriptorHeapDX12* heap : myDynamicShaderVisibleHeaps)
      if (heap != nullptr)
        heapsToBind[numHeapsToBind++] = heap->GetHeap();

    if (numHeapsToBind > 0u)
      myCommandList->SetDescriptorHeaps(numHeapsToBind, heapsToBind);
  }
  //---------------------------------------------------------------------------//
  void CommandContextBaseDX12::KickoffResourceBarriers()
  {
    if (myWaitingResourceBarriers.empty())
      return;

    myCommandList->ResourceBarrier(myWaitingResourceBarriers.size(), &myWaitingResourceBarriers[0]);
    myWaitingResourceBarriers.clear();
  }
  //---------------------------------------------------------------------------//
  void CommandContextBaseDX12::ReleaseAllocator(uint64 aFenceVal)
  {
    RenderCore::ReleaseCommandAllocator(myCommandAllocator, myCommandListType, aFenceVal);
    myCommandAllocator = nullptr;
  }
  //---------------------------------------------------------------------------//
  void CommandContextBaseDX12::ReleaseDynamicHeaps(uint64 aFenceVal)
  {
    for (DescriptorHeapDX12* heap : myDynamicShaderVisibleHeaps)
    {
      if (heap != nullptr)
        RenderCore::ReleaseDynamicDescriptorHeap(heap, myCommandListType, aFenceVal);
    }

    for (DescriptorHeapDX12* heap : myRetiredDescriptorHeaps)
    {
      RenderCore::ReleaseDynamicDescriptorHeap(heap, myCommandListType, aFenceVal);
    }

    myRetiredDescriptorHeaps.clear();
    memset(myDynamicShaderVisibleHeaps, 0, sizeof(myDynamicShaderVisibleHeaps));
  }
//---------------------------------------------------------------------------//
} } }
