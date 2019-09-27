#include "fancy_core_precompile.h"
#include "CommandListDX12.h"

#include "FancyCoreDefines.h"

#include "GpuBufferDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"
#include "TextureDX12.h"
#include "AdapterDX12.h"
#include "ShaderDX12.h"
#include "ShaderPipelineDX12.h"
#include "BlendState.h"
#include "DepthStencilState.h"
#include "GeometryData.h"
#include "GpuResourceDataDX12.h"
#include "GpuResourceViewDX12.h"
#include "TimeManager.h"
#include "GpuQueryHeapDX12.h"

namespace Fancy { 
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
          ASSERT(false, "CommandListType %d not implemented", (uint)aCommandListType);
          return D3D12_COMMAND_LIST_TYPE_DIRECT;
      }
    }
//---------------------------------------------------------------------------//
    const char* locResourceStatesToString(uint someStates, StaticString<2048>& aString)
    {
      if (someStates == 0)
      {
        aString.Format("COMMON/PRESENT");
        return aString;
      }
      if (someStates & D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
        aString.Append("VERTEX_AND_CONSTANT_BUFFER | ");
      if (someStates & D3D12_RESOURCE_STATE_INDEX_BUFFER)
        aString.Append("INDEX_BUFFER | ");
      if (someStates & D3D12_RESOURCE_STATE_RENDER_TARGET)
        aString.Append("RENDER_TARGET | ");
      if (someStates & D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
        aString.Append("UNORDERED_ACCESS | ");
      if (someStates & D3D12_RESOURCE_STATE_DEPTH_WRITE)
        aString.Append("DEPTH_WRITE | ");
      if (someStates & D3D12_RESOURCE_STATE_DEPTH_READ)
        aString.Append("DEPTH_READ | ");
      if (someStates & D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
        aString.Append("NON_PIXEL_SHADER_RESOURCE | ");
      if (someStates & D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
        aString.Append("PIXEL_SHADER_RESOURCE | ");
      if (someStates & D3D12_RESOURCE_STATE_STREAM_OUT)
        aString.Append("STREAM_OUT | ");
      if (someStates & D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT)
        aString.Append("INDIRECT_ARGUMENT | ");
      if (someStates & D3D12_RESOURCE_STATE_COPY_DEST)
        aString.Append("COPY_DEST | ");
      if (someStates & D3D12_RESOURCE_STATE_COPY_SOURCE)
        aString.Append("COPY_SOURCE | ");
      if (someStates & D3D12_RESOURCE_STATE_RESOLVE_DEST)
        aString.Append("RESOLVE_DEST | ");
      if (someStates & D3D12_RESOURCE_STATE_RESOLVE_SOURCE)
        aString.Append("RESOLVE_SOURCE | ");
      if (someStates & D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE)
        aString.Append("RT_ACCELERATION_STRUCTURE | ");
      
      return aString;
    }
//---------------------------------------------------------------------------//
#if FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES
    void locValidateUsageStatesCopy(const GpuResource* aDst, uint aDstSubresource, const GpuResource* aSrc, uint aSrcSubresource)
    {
      const GpuResourceStateTracking& src = aSrc->myStateTracking;
      const GpuResourceStateTracking& dst = aDst->myStateTracking;

      ASSERT(src.myDx12Data.mySubresourceStates[aSrcSubresource] & D3D12_RESOURCE_STATE_COPY_SOURCE);
      ASSERT(dst.myDx12Data.mySubresourceStates[aDstSubresource] & D3D12_RESOURCE_STATE_COPY_DEST);
    }
  //---------------------------------------------------------------------------//
    void locValidateUsageStatesCopy(const GpuResource* aDst, const GpuResource* aSrc)
    {
      const GpuResourceStateTracking& src = aSrc->myStateTracking;
      const GpuResourceStateTracking& dst = aDst->myStateTracking;

      for (uint i = 0u; i < (src.myAllSubresourcesInSameState ? 1u : src.myDx12Data.mySubresourceStates.size()); ++i)
         ASSERT(src.myDx12Data.mySubresourceStates[i] & D3D12_RESOURCE_STATE_COPY_SOURCE);

      for (uint i = 0u; i < (dst.myAllSubresourcesInSameState ? 1u : dst.myDx12Data.mySubresourceStates.size()); ++i)
         ASSERT(dst.myDx12Data.mySubresourceStates[i] & D3D12_RESOURCE_STATE_COPY_DEST);
    }
#endif
  //---------------------------------------------------------------------------//
  }
//---------------------------------------------------------------------------//
  std::unordered_map<uint64, ID3D12PipelineState*> CommandListDX12::ourPSOcache;
//---------------------------------------------------------------------------//
  CommandListDX12::CommandListDX12(CommandListType aCommandListType)
    : CommandList(aCommandListType)
    , myIsOpen(true)
    , myRootSignature(nullptr)
    , myComputeRootSignature(nullptr)
    , myCommandList(nullptr)
    , myCommandAllocator(nullptr)
    , myNumPendingBarriers(0u)
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
  CommandListDX12::~CommandListDX12()
  {
    CommandListDX12::ReleaseGpuResources(0ull);

    if (myCommandList != nullptr)
      myCommandList->Release();

    myCommandList = nullptr;

    if (myCommandAllocator != nullptr)
      RenderCore::GetPlatformDX12()->ReleaseCommandAllocator(myCommandAllocator, 0ull);
  }
//---------------------------------------------------------------------------//
  D3D12_DESCRIPTOR_HEAP_TYPE CommandListDX12::ResolveDescriptorHeapTypeFromMask(uint aDescriptorTypeMask)
  {
    if (aDescriptorTypeMask & (uint)GpuDescriptorTypeFlags::BUFFER_TEXTURE_CONSTANT_BUFFER)
      return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    else if (aDescriptorTypeMask & (uint)GpuDescriptorTypeFlags::SAMPLER)
      return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;

    ASSERT(false, "unsupported descriptor type mask");
    return (D3D12_DESCRIPTOR_HEAP_TYPE)-1;
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType, DynamicDescriptorHeapDX12* aDescriptorHeap)
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
  void CommandListDX12::UpdateSubresources(ID3D12Resource* aDestResource, ID3D12Resource* aStagingResource,
    uint aFirstSubresourceIndex, uint aNumSubresources, D3D12_SUBRESOURCE_DATA* someSubresourceDatas)
  {
    D3D12_RESOURCE_DESC srcDesc = aStagingResource->GetDesc();
    D3D12_RESOURCE_DESC destDesc = aDestResource->GetDesc();
    
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT* destLayouts = static_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(alloca(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) * aNumSubresources));
    uint64* destRowSizesByte = static_cast<uint64*>(alloca(sizeof(uint64) * aNumSubresources));
    uint* destRowNums = static_cast<uint*>(alloca(sizeof(uint) * aNumSubresources));

    uint64 destTotalSizeBytes = 0u;
    RenderCore::GetPlatformDX12()->GetDevice()->GetCopyableFootprints(&destDesc, aFirstSubresourceIndex, aNumSubresources, 0u, destLayouts, destRowNums, destRowSizesByte, &destTotalSizeBytes);

    FlushBarriers();

    // Prepare a temporary buffer that contains all subresource data in the expected form (i.e. respecting the dest data layout)
    uint8* tempBufferDataPtr;
    if (S_OK != aStagingResource->Map(0, nullptr, reinterpret_cast<void**>(&tempBufferDataPtr)))
      return;

    for (uint i = 0u; i < aNumSubresources; ++i)
    {
      uint8* dstSubResourceData = tempBufferDataPtr + destLayouts[i].Offset;
      uint64 dstSubResourceRowSize = destLayouts[i].Footprint.RowPitch;
      uint64 dstSubResourceSliceSize = dstSubResourceRowSize * destRowNums[i];

      uint8* srcSubResourceData = (uint8*) someSubresourceDatas[i].pData;
      uint64 srcSubResourceRowSize = someSubresourceDatas[i].RowPitch;
      uint64 srcSubResourceSliceSize = someSubresourceDatas[i].SlicePitch;

      for (uint iSlice = 0u; iSlice < destLayouts[i].Footprint.Depth; ++iSlice)
      {
        uint8* destSliceDataPtr = dstSubResourceData + dstSubResourceSliceSize * iSlice;
        const uint8* srcSliceDataPtr = srcSubResourceData + srcSubResourceSliceSize * iSlice;
        for (uint iRow = 0u; iRow < destRowNums[i]; ++iRow)
        {
          uint8* destDataPtr = destSliceDataPtr + dstSubResourceRowSize * iRow;
          const uint8* srcDataPtr = srcSliceDataPtr + srcSubResourceRowSize * iRow;

          memcpy(destDataPtr, srcDataPtr, destRowSizesByte[i]);
        }
      }
    }
    aStagingResource->Unmap(0, nullptr);

    // Copy from the temp staging buffer to the destination resource (could be buffer or texture)
    if (destDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
    {
      myCommandList->CopyBufferRegion(aDestResource, 0, aStagingResource, destLayouts[0].Offset, destLayouts[0].Footprint.Width);
    }
    else
    {
      for (uint i = 0u; i < aNumSubresources; ++i)
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
  void CommandListDX12::ApplyDescriptorHeaps()
  {
    ID3D12DescriptorHeap* heapsToBind[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    memset(heapsToBind, 0, sizeof(heapsToBind));
    uint numHeapsToBind = 0u;

    for (DynamicDescriptorHeapDX12* heap : myDynamicShaderVisibleHeaps)
      if (heap != nullptr)
        heapsToBind[numHeapsToBind++] = heap->GetHeap();

    if (numHeapsToBind > 0u)
      myCommandList->SetDescriptorHeaps(numHeapsToBind, heapsToBind);
  }
//---------------------------------------------------------------------------//
  DescriptorDX12 CommandListDX12::CopyDescriptorsToDynamicHeapRange(const DescriptorDX12* someResources, uint aResourceCount)
  {
    ASSERT(aResourceCount > 0u);

    const DescriptorDX12& firstDescriptor = someResources[0];
    RenderCore_PlatformDX12* platformDx12 = RenderCore::GetPlatformDX12();

    const D3D12_DESCRIPTOR_HEAP_TYPE heapType = firstDescriptor.myHeapType;
    DynamicDescriptorHeapDX12* dynamicHeap = myDynamicShaderVisibleHeaps[heapType];

    if (dynamicHeap == nullptr || dynamicHeap->GetNumFreeDescriptors() < aResourceCount)
    {
      dynamicHeap = platformDx12->AllocateDynamicDescriptorHeap(aResourceCount, heapType);
      SetDescriptorHeap(heapType, dynamicHeap);
    }

    DescriptorDX12 destRangeStartDescriptor = dynamicHeap->AllocateDescriptorRangeGetFirst(aResourceCount);

    const uint numSrcRanges = aResourceCount;
    uint* const srcRangeSizes = static_cast<uint*>(alloca(sizeof(uint) * numSrcRanges));
    D3D12_CPU_DESCRIPTOR_HANDLE* const srcDescriptors = static_cast<D3D12_CPU_DESCRIPTOR_HANDLE*>(alloca(sizeof(D3D12_CPU_DESCRIPTOR_HANDLE) * numSrcRanges));
    for (uint i = 0u; i < aResourceCount; ++i)
    {
      srcRangeSizes[i] = 1;
      srcDescriptors[i] = someResources[i].myCpuHandle;
    }

    platformDx12->GetDevice()->CopyDescriptors(
      1, &destRangeStartDescriptor.myCpuHandle, &aResourceCount,
      aResourceCount, srcDescriptors, srcRangeSizes, heapType);

    return destRangeStartDescriptor;
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::ClearRenderTarget(TextureView* aTextureView, const float* aColor)
  {
    const GpuResourceViewDataDX12& viewDataDx12 = aTextureView->myNativeData.To<GpuResourceViewDataDX12>();

    ASSERT(aTextureView->GetProperties().myIsRenderTarget);
    ASSERT(viewDataDx12.myType == GpuResourceViewDataDX12::RTV);

#if FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES
    SetTrackResourceTransitionBarrier(aTextureView->GetTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET);
#endif
    FlushBarriers();

    myCommandList->ClearRenderTargetView(viewDataDx12.myDescriptor.myCpuHandle, aColor, 0, nullptr);
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::ClearDepthStencilTarget(TextureView* aTextureView, float aDepthClear, uint8 aStencilClear, uint someClearFlags)
  {
    const GpuResourceViewDataDX12& viewDataDx12 = aTextureView->myNativeData.To<GpuResourceViewDataDX12>();
    ASSERT(viewDataDx12.myType == GpuResourceViewDataDX12::DSV);

#if FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES
    SetTrackResourceTransitionBarrier(aTextureView->GetTexture(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
#endif
    FlushBarriers();
    
    D3D12_CLEAR_FLAGS clearFlags = (D3D12_CLEAR_FLAGS)0;
    if (someClearFlags & (uint)DepthStencilClearFlags::CLEAR_DEPTH)
      clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
    if (someClearFlags & (uint)DepthStencilClearFlags::CLEAR_STENCIL)
      clearFlags |= D3D12_CLEAR_FLAG_STENCIL;

    myCommandList->ClearDepthStencilView(viewDataDx12.myDescriptor.myCpuHandle, clearFlags, aDepthClear, aStencilClear, 0, nullptr);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::CopyResource(GpuResource* aDestResource, GpuResource* aSrcResource)
  {
#if FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES
    const GpuResource* resourcesToTransition[] = { aDestResource, aSrcResource };
    const D3D12_RESOURCE_STATES barrierStates[] = { D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE };
    SetTrackResourceTransitionBarriers(resourcesToTransition, barrierStates, 2);
#endif
    FlushBarriers();

    GpuResourceDataDX12* destData = aDestResource->myNativeData.To<GpuResourceDataDX12*>();
    GpuResourceDataDX12* srcData = aSrcResource->myNativeData.To<GpuResourceDataDX12*>();

    myCommandList->CopyResource(destData->myResource.Get(), srcData->myResource.Get());
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::CopyBufferRegion(const GpuBuffer* aDestBuffer, uint64 aDestOffset, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset, uint64 aSize)
  {
    ASSERT(aDestBuffer != aSrcBuffer, "Copying within the same buffer is not supported (same subresource)");
    ASSERT(aSize <= aDestBuffer->GetByteSize() - aDestOffset, "Invalid dst-region specified");
    ASSERT(aSize <= aSrcBuffer->GetByteSize() - aSrcOffset, "Invalid src-region specified");

#if FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES
    const GpuResource* resourcesToTransition[] = { aDestBuffer, aSrcBuffer };
    const D3D12_RESOURCE_STATES barrierStates[] = { D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE };
    SetTrackResourceTransitionBarriers(resourcesToTransition, barrierStates, 2);
#endif
    FlushBarriers();

    ID3D12Resource* dstResource = static_cast<const GpuBufferDX12*>(aDestBuffer)->GetData()->myResource.Get();
    ID3D12Resource* srcResource = static_cast<const GpuBufferDX12*>(aSrcBuffer)->GetData()->myResource.Get();

    myCommandList->CopyBufferRegion(dstResource, aDestOffset, srcResource, aSrcOffset, aSize);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::CopyTextureRegion(const GpuBuffer* aDestBuffer, uint64 aDestOffset, const Texture* aSrcTexture, const TextureSubLocation& aSrcSubLocation, const TextureRegion* aSrcRegion)
  {
    ID3D12Resource* bufferResourceDX12 = static_cast<const GpuBufferDX12*>(aDestBuffer)->GetData()->myResource.Get();
    ID3D12Resource* textureResourceDX12 = static_cast<const TextureDX12*>(aSrcTexture)->GetData()->myResource.Get();

    const uint16 bufferSubresourceIndex = 0;
    const uint16 textureSubresourceIndex = static_cast<uint16>(aSrcTexture->GetSubresourceIndex(aSrcSubLocation));

#if FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES
    const GpuResource* resourcesToTransition[] = { aDestBuffer, aSrcTexture };
    const uint16* subresourceLists[] = { &bufferSubresourceIndex, &textureSubresourceIndex };
    const uint numSubresources[] = { 1, 1 };
    const D3D12_RESOURCE_STATES barrierStates[] = { D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE };
    SetTrackSubresourceTransitionBarriers(resourcesToTransition, barrierStates, subresourceLists, numSubresources, 2);
#endif

    ID3D12Device* device = RenderCore::GetPlatformDX12()->GetDevice();

    D3D12_TEXTURE_COPY_LOCATION srcLocation;
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    srcLocation.SubresourceIndex = textureSubresourceIndex;
    srcLocation.pResource = textureResourceDX12;

    const D3D12_RESOURCE_DESC& textureResourceDescDX12 = textureResourceDX12->GetDesc();
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
    uint numRows;
    uint64 rowSizeBytes;
    uint64 totalSizeBytes;
    device->GetCopyableFootprints(&textureResourceDescDX12, srcLocation.SubresourceIndex, 1u, 0u, &footprint, &numRows, &rowSizeBytes, &totalSizeBytes);

    ASSERT(totalSizeBytes <= aDestBuffer->GetByteSize() - aDestOffset);

    D3D12_TEXTURE_COPY_LOCATION destLocation;
    destLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    destLocation.pResource = bufferResourceDX12;
    destLocation.PlacedFootprint.Footprint = footprint.Footprint;
    destLocation.PlacedFootprint.Offset = aDestOffset + footprint.Offset;

    FlushBarriers();

    if (aSrcRegion != nullptr)
    {
      D3D12_BOX srcBox;
      srcBox.left = aSrcRegion->myTexelPos.x;
      srcBox.right = aSrcRegion->myTexelPos.x + aSrcRegion->myTexelSize.x;
      srcBox.top = aSrcRegion->myTexelPos.y;
      srcBox.bottom = aSrcRegion->myTexelPos.y + aSrcRegion->myTexelSize.y;
      srcBox.front = aSrcRegion->myTexelPos.z;
      srcBox.back = aSrcRegion->myTexelPos.z + aSrcRegion->myTexelSize.z;
      myCommandList->CopyTextureRegion(&destLocation, 0, 0, 0, &srcLocation, &srcBox);
    }
    else
    {
      myCommandList->CopyTextureRegion(&destLocation, 0, 0, 0, &srcLocation, nullptr);
    }
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::CopyTextureRegion(const Texture* aDestTexture, const TextureSubLocation& aDestSubLocation, const glm::uvec3& aDestTexelPos, const Texture* aSrcTexture, const TextureSubLocation& aSrcSubLocation, const TextureRegion* aSrcRegion /*= nullptr*/)
  {
    const TextureProperties& dstProps = aDestTexture->GetProperties();
    const TextureProperties& srcProps = aSrcTexture->GetProperties();

    // TODO: asserts and validation of regions against texture-parameters
    
    ID3D12Resource* dstResource = static_cast<const TextureDX12*>(aDestTexture)->GetData()->myResource.Get();
    ID3D12Resource* srcResource = static_cast<const TextureDX12*>(aSrcTexture)->GetData()->myResource.Get();

    const uint16 destSubResourceIndex = static_cast<uint16>(aDestTexture->GetSubresourceIndex(aDestSubLocation));
    const uint16 srcSubResourceIndex = static_cast<uint16>(aSrcTexture->GetSubresourceIndex(aSrcSubLocation));

#if FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES
    const GpuResource* resourcesToTransition[] = { aDestTexture, aSrcTexture };
    const uint16* subresourceLists[] = { &destSubResourceIndex, &srcSubResourceIndex };
    const uint numSubresources[] = { 1, 1 };
    const D3D12_RESOURCE_STATES barrierStates[] = { D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE };
    SetTrackSubresourceTransitionBarriers(resourcesToTransition, barrierStates, subresourceLists, numSubresources, 2);
#endif

    D3D12_TEXTURE_COPY_LOCATION dstLocation;
    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLocation.SubresourceIndex = destSubResourceIndex;
    dstLocation.pResource = dstResource;

    D3D12_TEXTURE_COPY_LOCATION srcLocation;
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    srcLocation.SubresourceIndex = srcSubResourceIndex;
    srcLocation.pResource = srcResource;

    FlushBarriers();

    if (aSrcRegion != nullptr)
    {
      D3D12_BOX srcBox;
      srcBox.left = aSrcRegion->myTexelPos.x;
      srcBox.right = aSrcRegion->myTexelPos.x + aSrcRegion->myTexelSize.x;
      srcBox.top = aSrcRegion->myTexelPos.y;
      srcBox.bottom = aSrcRegion->myTexelPos.y + aSrcRegion->myTexelSize.y;
      srcBox.front = aSrcRegion->myTexelPos.z;
      srcBox.back = aSrcRegion->myTexelPos.z + aSrcRegion->myTexelSize.z;
      myCommandList->CopyTextureRegion(&dstLocation, aDestTexelPos.x, aDestTexelPos.y, aDestTexelPos.z, &srcLocation, &srcBox);
    }
    else
    {
      myCommandList->CopyTextureRegion(&dstLocation, aDestTexelPos.x, aDestTexelPos.y, aDestTexelPos.z, &srcLocation, nullptr);
    }
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::CopyTextureRegion(const Texture* aDestTexture, const TextureSubLocation& aDestSubLocation, const glm::uvec3& aDestTexelPos, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset)
  {
    const TextureProperties& dstProps = aDestTexture->GetProperties();

    ID3D12Resource* dstResource = static_cast<const TextureDX12*>(aDestTexture)->GetData()->myResource.Get();
    ID3D12Resource* srcResource = static_cast<const GpuBufferDX12*>(aSrcBuffer)->GetData()->myResource.Get();

    const uint16 destSubResourceIndex = static_cast<uint16>(aDestTexture->GetSubresourceIndex(aDestSubLocation));
    const uint16 srcSubResourceIndex = 0;

#if FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES
    const GpuResource* resourcesToTransition[] = { aDestTexture, aSrcBuffer };
    const uint16* subresourceLists[] = { &destSubResourceIndex, &srcSubResourceIndex };
    const uint numSubresources[] = { 1, 1 };
    const D3D12_RESOURCE_STATES barrierStates[] = { D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE };
    SetTrackSubresourceTransitionBarriers(resourcesToTransition, barrierStates, subresourceLists, numSubresources, 2);
#endif
    
    D3D12_TEXTURE_COPY_LOCATION dstLocation;
    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLocation.SubresourceIndex = destSubResourceIndex;
    dstLocation.pResource = dstResource;

    ID3D12Device* device = RenderCore::GetPlatformDX12()->GetDevice();

    const D3D12_RESOURCE_DESC& dstResourceDesc = dstResource->GetDesc();

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
    uint numRows;
    uint64 rowSizeBytes;
    uint64 totalSizeBytes;
    device->GetCopyableFootprints(&dstResourceDesc, dstLocation.SubresourceIndex, 1u, 0u, &footprint, &numRows, &rowSizeBytes, &totalSizeBytes);
    
    D3D12_TEXTURE_COPY_LOCATION srcLocation;
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    srcLocation.pResource = srcResource;
    srcLocation.PlacedFootprint.Footprint = footprint.Footprint;
    srcLocation.PlacedFootprint.Offset = aSrcOffset + footprint.Offset;

    FlushBarriers();
    myCommandList->CopyTextureRegion(&dstLocation, aDestTexelPos.x, aDestTexelPos.y, aDestTexelPos.z, &srcLocation, nullptr);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::ReleaseGpuResources(uint64 aFenceVal)
  {
    CommandList::ReleaseGpuResources(aFenceVal);

    for (DynamicDescriptorHeapDX12* heap : myDynamicShaderVisibleHeaps)
      if (heap != nullptr)
        RenderCore::GetPlatformDX12()->ReleaseDynamicDescriptorHeap(heap, aFenceVal);
    memset(myDynamicShaderVisibleHeaps, 0, sizeof(myDynamicShaderVisibleHeaps));

    for (DynamicDescriptorHeapDX12* heap : myRetiredDescriptorHeaps)
      RenderCore::GetPlatformDX12()->ReleaseDynamicDescriptorHeap(heap, aFenceVal);
    myRetiredDescriptorHeaps.clear();

    if (myCommandAllocator != nullptr)
      RenderCore::GetPlatformDX12()->ReleaseCommandAllocator(myCommandAllocator, aFenceVal);
    myCommandAllocator = nullptr;
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::Reset()
  {
    CommandList::Reset();

    myCommandAllocator = RenderCore::GetPlatformDX12()->GetCommandAllocator(myCommandListType);
    ASSERT(myCommandAllocator != nullptr);
    
    CheckD3Dcall(myCommandList->Reset(myCommandAllocator, nullptr));

    myRootSignature = nullptr;
    myComputeRootSignature = nullptr;
    myIsOpen = true;
    myNumPendingBarriers = 0u;
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::FlushBarriers()
  {
    if (myNumPendingBarriers > 0)
    {
      myCommandList->ResourceBarrier(myNumPendingBarriers, myPendingBarriers);
      myNumPendingBarriers = 0u;
    }
  }
//---------------------------------------------------------------------------//
  D3D12_GRAPHICS_PIPELINE_STATE_DESC CommandListDX12::GetNativePSOdesc(const GraphicsPipelineState& aState)
  {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    memset(&psoDesc, 0u, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

    // SHADER BYTECODES
    D3D12_SHADER_BYTECODE* shaderDescs[]{ &psoDesc.VS, &psoDesc.PS, &psoDesc.DS, &psoDesc.HS, &psoDesc.GS };
    ASSERT(ARRAY_LENGTH(shaderDescs) == (uint)ShaderStage::NUM_NO_COMPUTE);

    if (aState.myShaderPipeline != nullptr)
    {
      for (uint i = 0u; i < (uint)ShaderStage::NUM_NO_COMPUTE; ++i)
      {
        if (nullptr == aState.myShaderPipeline->myShaders[i])
          continue;

        const ShaderDX12* shaderDx12 = static_cast<const ShaderDX12*>(aState.myShaderPipeline->myShaders[i].get());

        (*shaderDescs[i]) = shaderDx12->getNativeByteCode();
      }
    }

    // ROOT SIGNATURE
    const ShaderPipelineDX12* shaderPipelineDx12 = static_cast<const ShaderPipelineDX12*>(aState.myShaderPipeline.get());
    psoDesc.pRootSignature = shaderPipelineDx12->GetRootSignature();

    // BLEND DESC
    D3D12_BLEND_DESC& blendDesc = psoDesc.BlendState;
    memset(&blendDesc, 0u, sizeof(D3D12_BLEND_DESC));
    blendDesc.AlphaToCoverageEnable = aState.myBlendState->myAlphaToCoverageEnabled;
    blendDesc.IndependentBlendEnable = aState.myBlendState->myBlendStatePerRT;
    uint rtCount = blendDesc.IndependentBlendEnable ? RenderConstants::kMaxNumRenderTargets : 1u;
    for (uint rt = 0u; rt < rtCount; ++rt)
    {
      D3D12_RENDER_TARGET_BLEND_DESC& rtBlendDesc = blendDesc.RenderTarget[rt];
      memset(&rtBlendDesc, 0u, sizeof(D3D12_RENDER_TARGET_BLEND_DESC));
      rtBlendDesc.BlendEnable = aState.myBlendState->myBlendEnabled[rt];
      rtBlendDesc.BlendOp = Adapter::toNativeType(aState.myBlendState->myBlendOp[rt]);
      rtBlendDesc.DestBlend = Adapter::toNativeType(aState.myBlendState->myDestBlend[rt]);
      rtBlendDesc.SrcBlend = Adapter::toNativeType(aState.myBlendState->mySrcBlend[rt]);

      if (aState.myBlendState->myAlphaSeparateBlend[rt])
      {
        rtBlendDesc.BlendOpAlpha = Adapter::toNativeType(aState.myBlendState->myBlendOpAlpha[rt]);
        rtBlendDesc.DestBlendAlpha = Adapter::toNativeType(aState.myBlendState->myDestBlendAlpha[rt]);
        rtBlendDesc.SrcBlendAlpha = Adapter::toNativeType(aState.myBlendState->mySrcBlendAlpha[rt]);
      }
      else
      {
        rtBlendDesc.BlendOpAlpha = rtBlendDesc.BlendOp;
        rtBlendDesc.DestBlendAlpha = rtBlendDesc.DestBlend;
        rtBlendDesc.SrcBlendAlpha = rtBlendDesc.SrcBlend;
      }

      // FEATURE: Add support for LogicOps?
      rtBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
      rtBlendDesc.LogicOpEnable = false;

      if ((aState.myBlendState->myRTwriteMask[rt] & 0xFFFFFF) > 0u)
      {
        rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
      }
      else
      {
        const bool red = (aState.myBlendState->myRTwriteMask[rt] & 0xFF000000) > 0u;
        const bool green = (aState.myBlendState->myRTwriteMask[rt] & 0x00FF0000) > 0u;
        const bool blue = (aState.myBlendState->myRTwriteMask[rt] & 0x0000FF00) > 0u;
        const bool alpha = (aState.myBlendState->myRTwriteMask[rt] & 0x000000FF) > 0u;
        rtBlendDesc.RenderTargetWriteMask |= red ? D3D12_COLOR_WRITE_ENABLE_RED : 0u;
        rtBlendDesc.RenderTargetWriteMask |= green ? D3D12_COLOR_WRITE_ENABLE_GREEN : 0u;
        rtBlendDesc.RenderTargetWriteMask |= blue ? D3D12_COLOR_WRITE_ENABLE_BLUE : 0u;
        rtBlendDesc.RenderTargetWriteMask |= alpha ? D3D12_COLOR_WRITE_ENABLE_ALPHA : 0u;
      }
    }

    // STREAM OUTPUT
    // FEATURE: Add support for StreamOutput
    D3D12_STREAM_OUTPUT_DESC& streamOutDesc = psoDesc.StreamOutput;
    memset(&streamOutDesc, 0u, sizeof(D3D12_STREAM_OUTPUT_DESC));

    // SAMPLE MASK / DESC
    psoDesc.SampleMask = ~0u;
    psoDesc.SampleDesc.Count = 1u;

    // RASTERIZER STATE
    D3D12_RASTERIZER_DESC& rasterizerDesc = psoDesc.RasterizerState;
    memset(&rasterizerDesc, 0u, sizeof(D3D12_RASTERIZER_DESC));
    rasterizerDesc.AntialiasedLineEnable = false;
    rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    rasterizerDesc.FillMode = Adapter::toNativeType(aState.myFillMode);
    rasterizerDesc.CullMode = Adapter::toNativeType(aState.myCullMode);
    rasterizerDesc.MultisampleEnable = false;
    rasterizerDesc.FrontCounterClockwise = aState.myWindingOrder == WindingOrder::CCW;
    rasterizerDesc.DepthBias = 0;
    rasterizerDesc.DepthBiasClamp = 0;
    rasterizerDesc.SlopeScaledDepthBias = 0;
    rasterizerDesc.DepthClipEnable = false;

    // DEPTH STENCIL STATE
    D3D12_DEPTH_STENCIL_DESC& dsState = psoDesc.DepthStencilState;
    dsState.DepthEnable = aState.myDepthStencilState->myDepthTestEnabled;
    dsState.DepthWriteMask = aState.myDepthStencilState->myDepthWriteEnabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    dsState.DepthFunc = Adapter::toNativeType(aState.myDepthStencilState->myDepthCompFunc);
    dsState.StencilEnable = aState.myDepthStencilState->myStencilEnabled;
    dsState.StencilReadMask = static_cast<uint8>(aState.myDepthStencilState->myStencilReadMask);
    dsState.StencilWriteMask = static_cast<uint8>(aState.myDepthStencilState->myStencilWriteMask[0u]);
    // FrontFace
    {
      D3D12_DEPTH_STENCILOP_DESC& faceDesc = dsState.FrontFace;
      uint faceIdx = static_cast<uint>(FaceType::FRONT);
      faceDesc.StencilFunc = Adapter::toNativeType(aState.myDepthStencilState->myStencilCompFunc[faceIdx]);
      faceDesc.StencilDepthFailOp = Adapter::toNativeType(aState.myDepthStencilState->myStencilDepthFailOp[faceIdx]);
      faceDesc.StencilFailOp = Adapter::toNativeType(aState.myDepthStencilState->myStencilFailOp[faceIdx]);
      faceDesc.StencilPassOp = Adapter::toNativeType(aState.myDepthStencilState->myStencilPassOp[faceIdx]);
    }
    // BackFace
    {
      D3D12_DEPTH_STENCILOP_DESC& faceDesc = dsState.BackFace;
      uint faceIdx = static_cast<uint>(FaceType::BACK);
      faceDesc.StencilFunc = Adapter::toNativeType(aState.myDepthStencilState->myStencilCompFunc[faceIdx]);
      faceDesc.StencilDepthFailOp = Adapter::toNativeType(aState.myDepthStencilState->myStencilDepthFailOp[faceIdx]);
      faceDesc.StencilFailOp = Adapter::toNativeType(aState.myDepthStencilState->myStencilFailOp[faceIdx]);
      faceDesc.StencilPassOp = Adapter::toNativeType(aState.myDepthStencilState->myStencilPassOp[faceIdx]);
    }

    // INPUT LAYOUT

    if (aState.myShaderPipeline != nullptr &&
      aState.myShaderPipeline->myShaders[(uint)ShaderStage::VERTEX] != nullptr)
    {
      const ShaderDX12* vertexShader =
        static_cast<const ShaderDX12*>(aState.myShaderPipeline->myShaders[(uint)ShaderStage::VERTEX].get());

      D3D12_INPUT_LAYOUT_DESC& inputLayout = psoDesc.InputLayout;
      inputLayout.NumElements = vertexShader->GetNumNativeInputElements();
      inputLayout.pInputElementDescs = vertexShader->GetNativeInputElements();
    }

    // IB STRIP CUT VALUE
    psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

    // TOPOLOGY TYPE
    psoDesc.PrimitiveTopologyType = Adapter::ResolveTopologyType(aState.myTopologyType);

    // NUM RENDER TARGETS
    psoDesc.NumRenderTargets = aState.myNumRenderTargets;

    // RTV-FORMATS
    for (uint i = 0u; i < aState.myNumRenderTargets; ++i)
    {
      psoDesc.RTVFormats[i] = RenderCore_PlatformDX12::ResolveFormat(aState.myRTVformats[i]);
    }

    // DSV FORMAT
    psoDesc.DSVFormat = RenderCore_PlatformDX12::GetDepthStencilViewFormat(RenderCore_PlatformDX12::ResolveFormat(aState.myDSVformat));

    // NODE MASK
    psoDesc.NodeMask = 0u;

    return psoDesc;
  }
//---------------------------------------------------------------------------//
  D3D12_COMPUTE_PIPELINE_STATE_DESC CommandListDX12::GetNativePSOdesc(const ComputePipelineState& aState)
  {
    D3D12_COMPUTE_PIPELINE_STATE_DESC desc;
    memset(&desc, 0u, sizeof(desc));

    if (aState.myShader != nullptr)
    {
      const ShaderDX12* shaderDx12 = static_cast<const ShaderDX12*>(aState.myShader);

      desc.pRootSignature = shaderDx12->GetRootSignature();
      desc.CS = shaderDx12->getNativeByteCode();
    }

    desc.NodeMask = 0u;
    return desc;
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::BindBuffer(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someViewProperties, uint aRegisterIndex) const
  {
    ASSERT(myCurrentContext != CommandListType::Graphics || myRootSignature != nullptr);
    ASSERT(myCurrentContext != CommandListType::Compute || myComputeRootSignature != nullptr);
    
    GpuResourceDataDX12* storage = static_cast<const GpuBufferDX12*>(aBuffer)->GetData();
    ASSERT(storage->myResource != nullptr);
    
    const uint64 gpuVirtualAddress = storage->myResource->GetGPUVirtualAddress() + someViewProperties.myOffset;

    GpuResourceViewDataDX12::Type type = GpuResourceViewDataDX12::NONE;
    if (someViewProperties.myIsShaderWritable)
    {
      ASSERT(someViewProperties.myIsRaw || someViewProperties.myIsStructured, "D3D12 only supports raw or structured buffer SRVs/UAVs as root descriptor");
      type = GpuResourceViewDataDX12::UAV;
    }
    else if (someViewProperties.myIsConstantBuffer)
    {
      type = GpuResourceViewDataDX12::CBV;
    }
    else
    {
      ASSERT(someViewProperties.myIsRaw || someViewProperties.myIsStructured, "D3D12 only supports raw or structured buffer SRVs/UAVs as root descriptor");
      type = GpuResourceViewDataDX12::SRV;
    }

#if FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES
    D3D12_RESOURCE_STATES state;
    if (someViewProperties.myIsShaderWritable)
      state = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    else if (someViewProperties.myIsConstantBuffer)
      state = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    else
      state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

    SetTrackResourceTransitionBarrier(aBuffer, state);
#endif  // FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES
    
    switch (myCurrentContext)
    {
      case CommandListType::Graphics: 
      {
        switch (type)
        {
          case GpuResourceViewDataDX12::SRV: { myCommandList->SetGraphicsRootShaderResourceView(aRegisterIndex, gpuVirtualAddress); break; }
          case GpuResourceViewDataDX12::UAV: { myCommandList->SetGraphicsRootUnorderedAccessView(aRegisterIndex, gpuVirtualAddress); break; }
          case GpuResourceViewDataDX12::CBV: { myCommandList->SetGraphicsRootConstantBufferView(aRegisterIndex, gpuVirtualAddress); break; }
          default: { ASSERT(false); break; }
        }
      } break;
      case CommandListType::Compute: 
      {
        switch (type)
        {
          case GpuResourceViewDataDX12::SRV: { myCommandList->SetComputeRootShaderResourceView(aRegisterIndex, gpuVirtualAddress); break; }
          case GpuResourceViewDataDX12::UAV: { myCommandList->SetComputeRootUnorderedAccessView(aRegisterIndex, gpuVirtualAddress); break; }
          case GpuResourceViewDataDX12::CBV: { myCommandList->SetComputeRootConstantBufferView(aRegisterIndex, gpuVirtualAddress); break; }
          default: { ASSERT(false); break; }
        }
      } break;
      case CommandListType::DMA: break;
      case CommandListType::NUM: break;
      default: break;
    }
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::BindResourceSet(const GpuResourceView** someResourceViews, uint aResourceCount, uint aRegisterIndex)
  {
    ASSERT(myCurrentContext != CommandListType::Graphics || myRootSignature != nullptr);
    ASSERT(myCurrentContext != CommandListType::Compute || myComputeRootSignature != nullptr);

    DescriptorDX12* dx12Descriptors = (DescriptorDX12*)alloca(sizeof(DescriptorDX12) * aResourceCount);
    
    for (uint i = 0; i < aResourceCount; ++i)
    {
      ASSERT(someResourceViews[i]->myNativeData.HasType<GpuResourceViewDataDX12>());
      const GpuResourceViewDataDX12& resourceViewData = someResourceViews[i]->myNativeData.To<GpuResourceViewDataDX12>();
      dx12Descriptors[i] = resourceViewData.myDescriptor;
    }

#if FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES
    const GpuResource** resourcesToTransition = (const GpuResource**)alloca(sizeof(GpuResource*) * aResourceCount);
    const uint16** subresourceLists = (const uint16**)alloca(sizeof(uint16*) * aResourceCount);
    uint* numSubresources = (uint*)alloca(sizeof(uint) * aResourceCount);
    D3D12_RESOURCE_STATES* barrierStates = (D3D12_RESOURCE_STATES*)alloca(sizeof(D3D12_RESOURCE_STATES) * aResourceCount);
    for (uint i = 0; i < aResourceCount; ++i)
    {
      ASSERT(someResourceViews[i]->myNativeData.HasType<GpuResourceViewDataDX12>());
      const GpuResourceViewDataDX12& resourceViewData = someResourceViews[i]->myNativeData.To<GpuResourceViewDataDX12>();

      subresourceLists[i] = someResourceViews[i]->mySubresources->data();
      numSubresources[i] = static_cast<uint>(someResourceViews[i]->mySubresources->size());
      resourcesToTransition[i] = someResourceViews[i]->myResource.get();
      switch (resourceViewData.myType)
      {
        case GpuResourceViewDataDX12::CBV: 
          barrierStates[i] = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
          break;
        case GpuResourceViewDataDX12::SRV: 
          barrierStates[i] = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
          break;
        case GpuResourceViewDataDX12::UAV: 
          barrierStates[i] = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
          break;
        default: ASSERT(false);
      }
    }
    SetTrackSubresourceTransitionBarriers(resourcesToTransition, barrierStates, subresourceLists, numSubresources, aResourceCount);
#endif  // FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES

    FlushBarriers();  // D3D12 needs a barrier flush here so the debug layer doesn't complain about expecting static descriptors and data at this point

    const DescriptorDX12 dynamicRangeStartDescriptor = CopyDescriptorsToDynamicHeapRange(dx12Descriptors, aResourceCount);

    switch(myCurrentContext)
    {
      case CommandListType::Graphics: 
        myCommandList->SetGraphicsRootDescriptorTable(aRegisterIndex, dynamicRangeStartDescriptor.myGpuHandle);
        break;
      case CommandListType::Compute: 
        myCommandList->SetComputeRootDescriptorTable(aRegisterIndex, dynamicRangeStartDescriptor.myGpuHandle);
        break;
      case CommandListType::DMA: break;
      case CommandListType::NUM: break;
      default: break;
    }
  }
//---------------------------------------------------------------------------//
  GpuQuery CommandListDX12::BeginQuery(GpuQueryType aType)
  {
    ASSERT(aType != GpuQueryType::TIMESTAMP, "Timestamp-queries should be used with InsertTimestamp");

    const GpuQuery query = AllocateQuery(aType);
    GpuQueryHeap* heap = RenderCore::GetQueryHeap(aType);

    const GpuQueryHeapDX12* queryHeapDx12 = (const GpuQueryHeapDX12*) heap;
    const D3D12_QUERY_TYPE queryTypeDx12 = Adapter::ResolveQueryType(aType);

    myCommandList->BeginQuery(queryHeapDx12->myHeap.Get(), queryTypeDx12, query.myIndexInHeap);
    return query;
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::EndQuery(const GpuQuery& aQuery)
  {
    ASSERT(aQuery.myFrame == Time::ourFrameIdx);
    ASSERT(aQuery.myType != GpuQueryType::TIMESTAMP, "Timestamp-queries should be used with InsertTimestamp");
    ASSERT(aQuery.myIsOpen);

    aQuery.myIsOpen = false;

    const GpuQueryType queryType = aQuery.myType;
    GpuQueryHeap* heap = RenderCore::GetQueryHeap(queryType);

    const GpuQueryHeapDX12* queryHeapDx12 = (const GpuQueryHeapDX12*)heap;
    const D3D12_QUERY_TYPE queryTypeDx12 = Adapter::ResolveQueryType(queryType);

    myCommandList->EndQuery(queryHeapDx12->myHeap.Get(), queryTypeDx12, aQuery.myIndexInHeap);
  }
//---------------------------------------------------------------------------//
  GpuQuery CommandListDX12::InsertTimestamp()
  {
    const GpuQuery query = AllocateQuery(GpuQueryType::TIMESTAMP);
    query.myIsOpen = false;

    GpuQueryHeap* heap = RenderCore::GetQueryHeap(GpuQueryType::TIMESTAMP);
    const GpuQueryHeapDX12* queryHeapDx12 = (const GpuQueryHeapDX12*)heap;

    myCommandList->EndQuery(queryHeapDx12->myHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, query.myIndexInHeap);
    return query;
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::CopyQueryDataToBuffer(const GpuQueryHeap* aQueryHeap, const GpuBuffer* aBuffer, uint aFirstQueryIndex, uint aNumQueries, uint64 aBufferOffset)
  {
    const GpuQueryHeapDX12* queryHeapDx12 = (const GpuQueryHeapDX12*)aQueryHeap;
    const GpuBufferDX12* bufferDx12 = (const GpuBufferDX12*)aBuffer;
    GpuResourceDataDX12* resourceDataDx12 = bufferDx12->GetData();

#if FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES
    SetTrackResourceTransitionBarrier(aBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
#endif

    myCommandList->ResolveQueryData(
      queryHeapDx12->myHeap.Get(),
      Adapter::ResolveQueryType(aQueryHeap->myType),
      aFirstQueryIndex,
      aNumQueries,
      bufferDx12->GetData()->myResource.Get(), 
      aBufferOffset);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::ResourceUAVbarrier(const GpuResource** someResources, uint aNumResources)
  {
    if (someResources == nullptr || aNumResources == 0u)
    {
      D3D12_RESOURCE_BARRIER barrier;
      barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
      barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
      barrier.UAV.pResource = nullptr;

      if (myNumPendingBarriers == ARRAY_LENGTH(myPendingBarriers))
        FlushBarriers();

      myPendingBarriers[myNumPendingBarriers++] = barrier;
    }
    else
    {
      D3D12_RESOURCE_BARRIER* barriers = (D3D12_RESOURCE_BARRIER*) alloca(sizeof(D3D12_RESOURCE_BARRIER) * aNumResources);
      for (uint iRes = 0u; iRes < aNumResources; ++iRes)
      {
        const GpuResource* resource = someResources[iRes];
        ID3D12Resource* resourceDx12 = resource->myNativeData.To<GpuResourceDataDX12*>()->myResource.Get();

        D3D12_RESOURCE_BARRIER& barrier = barriers[iRes];
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrier.UAV.pResource = resourceDx12;

        if (myNumPendingBarriers == ARRAY_LENGTH(myPendingBarriers))
          FlushBarriers();

        myPendingBarriers[myNumPendingBarriers++] = barrier;
      }
    }
  }
//---------------------------------------------------------------------------//
  bool CommandListDX12::SubresourceBarrierInternal(
    const GpuResource* aResource, 
    const uint16* someSubresources,
    uint aNumSubresources, 
    GpuResourceState aSrcState, 
    GpuResourceState aDstState, 
    CommandListType aSrcQueue,
    CommandListType aDstQueue)
  {
    const uint stateMaskFrom = aSrcQueue == CommandListType::Graphics ? kResourceStateMask_GraphicsContext : kResourceStateMask_ComputeContext;
    const uint stateMaskTo = aDstQueue == CommandListType::Graphics ? kResourceStateMask_GraphicsContext : kResourceStateMask_ComputeContext;
    const uint stateMaskCmdList = myCommandListType == CommandListType::Graphics ? kResourceStateMask_GraphicsContext : kResourceStateMask_ComputeContext;

    D3D12_RESOURCE_BARRIER barriers[128];
    ASSERT(ARRAY_LENGTH(barriers) < ARRAY_LENGTH(myPendingBarriers), "Invalid barrier array sizes");

    uint numBarriers = 0;
    ID3D12Resource* resourceDx12 = aResource->myNativeData.To<GpuResourceDataDX12*>()->myResource.Get();
    GpuResourceStateTracking& resourceStateTracking = aResource->myStateTracking;
    if (!resourceStateTracking.myCanChangeStates)
      return false;

    const bool srcIsRead = aSrcState >= GpuResourceState::FIRST_READ_STATE && aSrcState <= GpuResourceState::LAST_READ_STATE;
    const bool dstIsRead = aDstState >= GpuResourceState::FIRST_READ_STATE && aDstState <= GpuResourceState::LAST_READ_STATE;

    uint srcStateDx12 = RenderCore_PlatformDX12::ResolveResourceUsageState(aSrcState);
    uint dstStateDx12 = RenderCore_PlatformDX12::ResolveResourceUsageState(aDstState);

    const uint resourceWriteStateMask = resourceStateTracking.myDx12Data.myWriteStates;
    const uint resourceReadStateMask = resourceStateTracking.myDx12Data.myReadStates;
    const bool srcWas0 = srcStateDx12 == 0;
    const bool dstWas0 = dstStateDx12 == 0;
    const uint allowedSrcStateDx12 = srcStateDx12 & (srcIsRead ? resourceReadStateMask : resourceWriteStateMask);
    const uint allowedDstStateDx12 = dstStateDx12 & (dstIsRead ? resourceReadStateMask : resourceWriteStateMask);
    srcStateDx12 = allowedSrcStateDx12 & stateMaskFrom & stateMaskCmdList;
    dstStateDx12 = allowedDstStateDx12 & stateMaskTo & stateMaskCmdList;
    ASSERT((srcWas0 || srcStateDx12 != 0) && (dstWas0 || dstStateDx12 != 0));

    if (someSubresources == nullptr || aNumSubresources >= aResource->myNumSubresources)  // Note: This would break if there are duplicated subresrouces in the list, but this wouldn't make any sense
    {
      ASSERT(numBarriers < ARRAY_LENGTH(barriers));
      D3D12_RESOURCE_BARRIER& barrier = barriers[numBarriers++];

#if  FANCY_RENDERER_LOG_RESOURCE_BARRIERS
      StaticString<2048> strBufFrom;
      StaticString<2048> strBufTo;
      LOG_INFO("DX12 resource barrier: %s from %s to %s", aResource->myName.c_str(), locResourceStatesToString(srcStateDx12, strBufFrom), locResourceStatesToString(dstStateDx12, strBufTo));
#endif
      barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
      barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
      barrier.Transition.StateBefore = (D3D12_RESOURCE_STATES)srcStateDx12;
      barrier.Transition.StateAfter = (D3D12_RESOURCE_STATES)dstStateDx12;
      barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
      barrier.Transition.pResource = resourceDx12;
    }
    else
    {
      for (uint iSub = 0u; iSub < aNumSubresources; ++iSub)
      {
        const uint subresourceIndex = static_cast<uint>(someSubresources[iSub]);

        ASSERT(numBarriers < ARRAY_LENGTH(barriers));
        D3D12_RESOURCE_BARRIER& barrier = barriers[numBarriers++];

#if  FANCY_RENDERER_LOG_RESOURCE_BARRIERS
        StaticString<2048> strBufFrom;
        StaticString<2048> strBufTo;
        LOG_INFO("DX12 subresource barrier: %s (subresource %d) from %s to %s", aResource->myName.c_str(), subresourceIndex, locResourceStatesToString(srcStateDx12, strBufFrom), locResourceStatesToString(dstStateDx12, strBufTo));
#endif
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.StateBefore = (D3D12_RESOURCE_STATES)srcStateDx12;
        barrier.Transition.StateAfter = (D3D12_RESOURCE_STATES)dstStateDx12;
        barrier.Transition.Subresource = subresourceIndex;
        barrier.Transition.pResource = resourceDx12;
      }
    }

    const uint maxNumPendingBarriers = ARRAY_LENGTH(myPendingBarriers);
    const uint numFreePendingBarriers = maxNumPendingBarriers - myNumPendingBarriers;
    if (numBarriers > numFreePendingBarriers)
      FlushBarriers();

    memcpy(myPendingBarriers + myNumPendingBarriers, barriers, sizeof(D3D12_RESOURCE_BARRIER) * numBarriers);
    myNumPendingBarriers += numBarriers;

    return true;
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::SetShaderPipeline(const SharedPtr<ShaderPipeline>& aShaderPipeline)
  {
    CommandList::SetShaderPipeline(aShaderPipeline);

    const ShaderPipelineDX12* pipelineDx12 = static_cast<const ShaderPipelineDX12*>(aShaderPipeline.get());
    ID3D12RootSignature* rootSignature = pipelineDx12->GetRootSignature();

    if (myRootSignature != rootSignature)
    {
      myRootSignature = rootSignature;
      myCommandList->SetGraphicsRootSignature(rootSignature);
    }
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::BindVertexBuffer(const GpuBuffer* aBuffer, uint aVertexSize, uint64 anOffset /*= 0u*/, uint64 aSize /*= ~0ULL*/)
  {
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

    GpuResourceDataDX12* storage = static_cast<const GpuBufferDX12*>(aBuffer)->GetData();

    uint64 resourceStartAddress = storage->myResource->GetGPUVirtualAddress();
    vertexBufferView.BufferLocation = resourceStartAddress + anOffset;

    const uint64 byteSize = glm::min(aSize, aBuffer->GetByteSize());
    ASSERT(byteSize <= UINT_MAX);

    vertexBufferView.SizeInBytes = static_cast<uint>(byteSize);
    vertexBufferView.StrideInBytes = static_cast<uint>(aVertexSize);

#if FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES
    SetTrackResourceTransitionBarrier(aBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
#endif

    // TODO: Don't set the primitive topology here: Changing the topology after binding the vertex buffer won't work otherwise...
    myCommandList->IASetPrimitiveTopology(Adapter::ResolveTopology(myGraphicsPipelineState.myTopologyType));
    myCommandList->IASetVertexBuffers(0, 1, &vertexBufferView);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::BindIndexBuffer(const GpuBuffer* aBuffer, uint anIndexSize, uint64 anIndexOffset /* = 0u */, uint64 aNumIndices /* =~0ULL*/)
  {
    ASSERT(anIndexSize == 2u || anIndexSize == 4u);

    D3D12_INDEX_BUFFER_VIEW indexBufferView;

    GpuResourceDataDX12* storage = static_cast<const GpuBufferDX12*>(aBuffer)->GetData();

    uint64 resourceStartAddress = storage->myResource->GetGPUVirtualAddress();
    indexBufferView.BufferLocation = resourceStartAddress + anIndexOffset;

    indexBufferView.Format = anIndexSize == 2u ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

    const uint64 byteSize = glm::min(aNumIndices, aBuffer->GetByteSize());
    
    ASSERT(byteSize <= UINT_MAX);
    indexBufferView.SizeInBytes = static_cast<uint>(byteSize);

#if FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES
    SetTrackResourceTransitionBarrier(aBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
#endif // FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES
    
    myCommandList->IASetIndexBuffer(&indexBufferView);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::Render(uint aNumIndicesPerInstance, uint aNumInstances, uint aStartIndex, uint aBaseVertex, uint aStartInstance)
  {
    FlushBarriers();
    ApplyViewportAndClipRect();
    ApplyRenderTargets();
    ApplyTopologyType();
    ApplyGraphicsPipelineState();

    myCommandList->DrawIndexedInstanced(aNumIndicesPerInstance, aNumInstances, aStartIndex, aBaseVertex, aStartInstance);
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::RenderGeometry(const GeometryData* pGeometry)
  {
    const GpuBufferDX12* vertexBufferDx12 = static_cast<const GpuBufferDX12*>(pGeometry->getVertexBuffer());
    const GpuBufferDX12* indexBufferDx12 = static_cast<const GpuBufferDX12*>(pGeometry->getIndexBuffer());

    ASSERT(vertexBufferDx12->GetProperties().myElementSizeBytes <= UINT_MAX);
    ASSERT(indexBufferDx12->GetProperties().myElementSizeBytes <= UINT_MAX);

    SetTopologyType(pGeometry->getGeometryVertexLayout().myTopology);
    BindVertexBuffer(pGeometry->getVertexBuffer(), (uint) vertexBufferDx12->GetProperties().myElementSizeBytes);
    BindIndexBuffer(pGeometry->getIndexBuffer(), (uint) indexBufferDx12->GetProperties().myElementSizeBytes);

    ASSERT(indexBufferDx12->GetProperties().myNumElements <= UINT_MAX);
    Render((uint) indexBufferDx12->GetProperties().myNumElements, 1, 0, 0, 0);
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::ApplyViewportAndClipRect()
  {
    if (myViewportDirty)
    {
      D3D12_VIEWPORT viewport = { 0u };
      viewport.TopLeftX = static_cast<float>(myViewportParams.x);
      viewport.TopLeftY = static_cast<float>(myViewportParams.y);
      viewport.Width = static_cast<float>(myViewportParams.z);
      viewport.Height =static_cast<float>(myViewportParams.w);
      viewport.MinDepth = 0.0f;
      viewport.MaxDepth = 1.0f;

      myCommandList->RSSetViewports(1u, &viewport);

      myClipRectDirty = true;
      myViewportDirty = false;
    }

    if (myClipRectDirty)
    {
      D3D12_RECT rect = { 0u };
      rect.left = myClipRect.x;
      rect.top = myClipRect.y;
      rect.right = myClipRect.z;
      rect.bottom = myClipRect.w;

      myCommandList->RSSetScissorRects(1u, &rect);

      myClipRectDirty = false;
    }
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::ApplyRenderTargets()
  {
    if (!myRenderTargetsDirty)
      return;

    const uint numRtsToSet = myGraphicsPipelineState.myNumRenderTargets;
    D3D12_CPU_DESCRIPTOR_HANDLE rtDescriptors[RenderConstants::kMaxNumRenderTargets];

    // RenderTarget state-transitions
    {
      const GpuResource* rtResources[RenderConstants::kMaxNumRenderTargets];
      const uint16* subresourceLists[RenderConstants::kMaxNumRenderTargets];
      uint numSubresources[RenderConstants::kMaxNumRenderTargets];

      for (uint i = 0u; i < numRtsToSet; ++i)
      {
        ASSERT(myRenderTargets[i] != nullptr);

        const GpuResourceViewDataDX12& viewData = myRenderTargets[i]->myNativeData.To<GpuResourceViewDataDX12>();
        ASSERT(viewData.myType == GpuResourceViewDataDX12::RTV);

        rtResources[i] = myRenderTargets[i]->GetTexture();
        rtDescriptors[i] = viewData.myDescriptor.myCpuHandle;
        subresourceLists[i] = myRenderTargets[i]->mySubresources[0].data();
        numSubresources[i] = static_cast<uint>(myRenderTargets[i]->mySubresources[0].size());
      }

#if FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES
      D3D12_RESOURCE_STATES newStates[RenderConstants::kMaxNumRenderTargets];
      for (uint i = 0; i < numRtsToSet; ++i)
        newStates[i] = D3D12_RESOURCE_STATE_RENDER_TARGET;

      SetTrackSubresourceTransitionBarriers(rtResources, newStates, subresourceLists, numSubresources, numRtsToSet);
#endif  // FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES
    }
    
    if (myDepthStencilTarget != nullptr)
    {
#if FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES
      const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(myDepthStencilTarget->GetTexture()->GetProperties().eFormat);
      const TextureViewProperties& dsvProps = myDepthStencilTarget->GetProperties();

      // TODO: Also respect stencilWriteMask per rendertarget?
      D3D12_RESOURCE_STATES dsStates[2] = { 
          (dsvProps.myIsDepthReadOnly || !myGraphicsPipelineState.myDepthStencilState->myDepthWriteEnabled) ? D3D12_RESOURCE_STATE_DEPTH_READ : D3D12_RESOURCE_STATE_DEPTH_WRITE,
          (dsvProps.myIsStencilReadOnly || !myGraphicsPipelineState.myDepthStencilState->myStencilEnabled) ? D3D12_RESOURCE_STATE_DEPTH_READ : D3D12_RESOURCE_STATE_DEPTH_WRITE
      };

      const GpuResource* resources[2] = { myDepthStencilTarget->GetTexture(), myDepthStencilTarget->GetTexture() };
      const uint16* subresourceLists[2] = { myDepthStencilTarget->mySubresources[0].data(), myDepthStencilTarget->mySubresources[1].data() };
      const uint numSubresources[2] = { static_cast<uint>(myDepthStencilTarget->mySubresources[0].size()), static_cast<uint>(myDepthStencilTarget->mySubresources[1].size()) };
      
      SetTrackSubresourceTransitionBarriers(resources, dsStates, subresourceLists, numSubresources, 2u);
#endif  // FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES
      
      const GpuResourceViewDataDX12& dsvViewData = myDepthStencilTarget->myNativeData.To<GpuResourceViewDataDX12>();
      ASSERT(dsvViewData.myType == GpuResourceViewDataDX12::DSV);
      myCommandList->OMSetRenderTargets(numRtsToSet, rtDescriptors, false, &dsvViewData.myDescriptor.myCpuHandle);
    }
    else
    {
      myCommandList->OMSetRenderTargets(numRtsToSet, rtDescriptors, false, nullptr);
    }

    myRenderTargetsDirty = false;
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::ApplyTopologyType()
  {
    if (!myTopologyDirty)
      return;

    myTopologyDirty = false;
    myCommandList->IASetPrimitiveTopology(Adapter::ResolveTopology(myGraphicsPipelineState.myTopologyType));
  }
//---------------------------------------------------------------------------//
#if FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES
  void CommandListDX12::SetTrackResourceTransitionBarrier(const GpuResource* aResource, D3D12_RESOURCE_STATES aNewState) const
  {
    SetTrackResourceTransitionBarriers(&aResource, &aNewState, 1u);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::SetTrackResourceTransitionBarriers(const GpuResource** someResources, const D3D12_RESOURCE_STATES* someNewStates, uint aNumResources) const
  {
    SetTrackSubresourceTransitionBarriers(someResources, someNewStates, nullptr, nullptr, aNumResources);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::SetTrackSubresourceTransitionBarrier(const GpuResource* aResource, uint16 aSubresourceIndex, D3D12_RESOURCE_STATES aNewState) const
  {
    const uint numSubresources = 1u;
    const uint16* subresourceLists[] = { &aSubresourceIndex };
    SetTrackSubresourceTransitionBarriers(&aResource, &aNewState, subresourceLists, &numSubresources, 1u);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::SetTrackSubresourceTransitionBarriers(const GpuResource** someResources, const D3D12_RESOURCE_STATES* someNewStates, const uint16** someSubresourceLists, const uint* someNumSubresources, uint aNumStates) const
  {
    D3D12_RESOURCE_BARRIER barriers[256];

    uint numBarriers = 0u;
    for (uint iState = 0u; iState < aNumStates; ++iState)
    {
      const D3D12_RESOURCE_STATES newState = someNewStates[iState];

      GpuResourceStateTracking& hazardData = someResources[iState]->myStateTracking;
      
      if (!hazardData.myCanChangeStates)
        continue;

      const uint maxNumSubresources = static_cast<uint>(hazardData.myDx12Data.mySubresourceStates.size());
      const uint numSubresources = someNumSubresources == nullptr ? maxNumSubresources : someNumSubresources[iState];
      const bool transitionAllSubresources = numSubresources == maxNumSubresources;

      auto TransitionSubresource = [&](uint iSub, bool aTransitionAllSubresources)
      {
        const D3D12_RESOURCE_STATES oldState = (D3D12_RESOURCE_STATES) hazardData.myDx12Data.mySubresourceStates[iSub];
        const CommandListType oldContext = hazardData.mySubresourceContexts[iSub];

        // Transition between DMA and Graphics/Compute: Common-state required
        if ((oldContext == CommandListType::DMA && myCommandListType != CommandListType::DMA)
          || (oldContext != CommandListType::DMA && myCommandListType == CommandListType::DMA))
          ASSERT(newState == D3D12_RESOURCE_STATE_COMMON, "Resource needs to be in the COMMON-state when switching between Graphics/Compute and DMA");

        hazardData.mySubresourceContexts[iSub] = myCommandListType;

        if (oldState == newState)
          return;

        // Validation to ensure the current context can understand the transition
        ASSERT(myCommandListType != CommandListType::Graphics || (oldState & kResourceStateMask_GraphicsContext) == oldState);
        ASSERT(myCommandListType != CommandListType::Graphics || (newState & kResourceStateMask_GraphicsContext) == newState);

        ASSERT(myCommandListType != CommandListType::Compute || (oldState & kResourceStateMask_ComputeContext) == oldState);
        ASSERT(myCommandListType != CommandListType::Compute || (newState & kResourceStateMask_ComputeContext) == newState);

        ASSERT(myCommandListType != CommandListType::DMA || (oldState & kResourceStateMask_Copy) == oldState);
        ASSERT(myCommandListType != CommandListType::DMA || (newState & kResourceStateMask_Copy) == newState);

        ASSERT(numBarriers < ARRAY_LENGTH(barriers), "Maximum expected number of barriers encountered. please increase the barrier array size");
        D3D12_RESOURCE_BARRIER& barrier = barriers[numBarriers++];
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

        GpuResourceDataDX12* resource = someResources[iState]->myNativeData.To<GpuResourceDataDX12*>();
        barrier.Transition.pResource = resource->myResource.Get();
        barrier.Transition.StateBefore = oldState;
        barrier.Transition.StateAfter = newState;
        
        if (aTransitionAllSubresources)
        {
          barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        
          for (uint i = 0u; i < hazardData.myDx12Data.mySubresourceStates.size(); ++i)
            hazardData.myDx12Data.mySubresourceStates[i] = newState;

          for (uint i = 0u; i < hazardData.mySubresourceContexts.size(); ++i)
            hazardData.mySubresourceContexts[i] = myCommandListType;

          hazardData.myAllSubresourcesInSameState = true;
        }
        else
        {
          barrier.Transition.Subresource = iSub;
          hazardData.myDx12Data.mySubresourceStates[iSub] = newState;
          hazardData.mySubresourceContexts[iSub] = myCommandListType;
        }
      };

      if (transitionAllSubresources)
      {
        if (hazardData.myAllSubresourcesInSameState)
        {
          TransitionSubresource(0, true);
        }

        for (uint iSub = 0u; iSub < maxNumSubresources; ++iSub)
        {
          TransitionSubresource(iSub, false);
        }
      }
      else
      {
        const uint16* subresourceList = someSubresourceLists[iState];
        for (uint i = 0u; i < someNumSubresources[iState]; ++i)
        {
          const uint iSub = subresourceList[i];
          TransitionSubresource(iSub, false);
        }
      }  

      if (!transitionAllSubresources)
      {
        bool sameSubresourceStates = true;
        const D3D12_RESOURCE_STATES firstSubresourceState = (D3D12_RESOURCE_STATES) hazardData.myDx12Data.mySubresourceStates[0u];
        for (uint subState : hazardData.myDx12Data.mySubresourceStates)
          sameSubresourceStates &= subState == firstSubresourceState;

        const CommandListType firstSubresourceContext = hazardData.mySubresourceContexts[0u];
        for (CommandListType subContext : hazardData.mySubresourceContexts)
          sameSubresourceStates &= subContext == firstSubresourceContext;

        hazardData.myAllSubresourcesInSameState = sameSubresourceStates;
      }
    }
    
    if (numBarriers > 0)
      myCommandList->ResourceBarrier(numBarriers, barriers);
  }
#endif  // FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES
//---------------------------------------------------------------------------//
  void CommandListDX12::ApplyGraphicsPipelineState()
  {
    if (!myGraphicsPipelineState.myIsDirty)
      return;

    myGraphicsPipelineState.myIsDirty = false;

    const uint64 requestedHash = myGraphicsPipelineState.GetHash();

    ID3D12PipelineState* pso = nullptr;

    const auto cachedPSOIter = ourPSOcache.find(requestedHash);
    if (cachedPSOIter != ourPSOcache.end())
    {
      pso = cachedPSOIter->second;
    }
    else
    {
      const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc = GetNativePSOdesc(myGraphicsPipelineState);
      const HRESULT result = RenderCore::GetPlatformDX12()->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));
      ASSERT(result == S_OK, "Error creating graphics PSO");

      ourPSOcache[requestedHash] = pso;
    }
    myCommandList->SetPipelineState(pso);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::ApplyComputePipelineState()
  {
    if (!myComputePipelineState.myIsDirty)
      return;

    myComputePipelineState.myIsDirty = false;

    const uint64 requestedHash = myComputePipelineState.GetHash();
    
    ID3D12PipelineState* pso = nullptr;

    const auto cachedPSOIter = ourPSOcache.find(requestedHash);
    if (cachedPSOIter != ourPSOcache.end())
    {
      pso = cachedPSOIter->second;
    }
    else
    {
      const D3D12_COMPUTE_PIPELINE_STATE_DESC& psoDesc = GetNativePSOdesc(myComputePipelineState);
      HRESULT result = RenderCore::GetPlatformDX12()->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pso));
      ASSERT(result == S_OK, "Error creating compute PSO");

      ourPSOcache[requestedHash] = pso;
    }
    myCommandList->SetPipelineState(pso);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::SetComputeProgram(const Shader* aProgram)
  {
    CommandList::SetComputeProgram(aProgram);

    const ShaderDX12* programDx12 = static_cast<const ShaderDX12*>(aProgram);

    if (myComputeRootSignature != programDx12->GetRootSignature())
    {
      myComputeRootSignature = programDx12->GetRootSignature();
      myCommandList->SetComputeRootSignature(myComputeRootSignature);
    }
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::Dispatch(const glm::int3& aNumThreads)
  {
    FlushBarriers();

    ApplyComputePipelineState();
    ASSERT(myComputePipelineState.myShader != nullptr);

    const glm::int3& numGroupThreads = myComputePipelineState.myShader->myProperties.myNumGroupThreads;
    const glm::int3 numGroups = glm::max(glm::int3(1), aNumThreads / numGroupThreads);
    myCommandList->Dispatch(static_cast<uint>(numGroups.x), static_cast<uint>(numGroups.y), static_cast<uint>(numGroups.z));
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::Close()
  {
    if (myIsOpen)
      CheckD3Dcall(myCommandList->Close());

    myIsOpen = false;
  }
//---------------------------------------------------------------------------//
}
