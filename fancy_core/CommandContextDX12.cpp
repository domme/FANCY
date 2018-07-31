#include "FancyCorePrerequisites.h"
#include "CommandContextDX12.h"
#include "GpuBufferDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"
#include "TextureDX12.h"
#include <malloc.h>
#include "AdapterDX12.h"
#include "GpuProgramDX12.h"
#include "ShaderResourceInterfaceDX12.h"
#include "GpuProgramPipeline.h"
#include "BlendState.h"
#include "DepthStencilState.h"
#include "GeometryData.h"
#include "GpuResourceStorageDX12.h"
#include "GpuResourceViewDX12.h"

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
          ASSERT(false, "CommandListType % not implemented", (uint)aCommandListType);
          return D3D12_COMMAND_LIST_TYPE_DIRECT;
      }
    }
  //---------------------------------------------------------------------------//
    bool locValidateUsageStatesCopy(const GpuResource* aDst, uint aDstSubresource, const GpuResource* aSrc, uint aSrcSubresource)
    {
      bool valid = (aSrc->myUsageState == GpuResourceState::RESOURCE_STATE_COPY_SRC || aSrc->myUsageState == GpuResourceState::RESOURCE_STATE_GENERIC_READ) &&
        (aDst->myUsageState == GpuResourceState::RESOURCE_STATE_COPY_DEST);
      ASSERT(valid);
      return valid;
    }
  //---------------------------------------------------------------------------//
    bool locValidateUsageStatesCopy(const GpuResource* aDst, const GpuResource* aSrc)
    {
      bool valid = (aSrc->myUsageState == GpuResourceState::RESOURCE_STATE_COPY_SRC || aSrc->myUsageState == GpuResourceState::RESOURCE_STATE_GENERIC_READ) &&
        (aDst->myUsageState == GpuResourceState::RESOURCE_STATE_COPY_DEST);
      ASSERT(valid);
      return valid;
    }
  //---------------------------------------------------------------------------//
  }
//---------------------------------------------------------------------------//
  std::unordered_map<uint64, ID3D12PipelineState*> CommandContextDX12::ourPSOcache;
//---------------------------------------------------------------------------//
  CommandContextDX12::CommandContextDX12(CommandListType aCommandListType)
    : CommandContext(aCommandListType)
    , myRootSignature(nullptr)
    , myCommandList(nullptr)
    , myCommandAllocator(nullptr)
    , myCommandListIsClosed(false)
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
  CommandContextDX12::~CommandContextDX12()
  {
    CommandContextDX12::Reset(0);

    if (myCommandList != nullptr)
      myCommandList->Release();

    myCommandList = nullptr;

    if (myCommandAllocator != nullptr)
      ReleaseAllocator(0u);
  }
//---------------------------------------------------------------------------//
  D3D12_DESCRIPTOR_HEAP_TYPE CommandContextDX12::ResolveDescriptorHeapTypeFromMask(uint aDescriptorTypeMask)
  {
    if (aDescriptorTypeMask & (uint)GpuDescriptorTypeFlags::BUFFER_TEXTURE_CONSTANT_BUFFER)
      return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    else if (aDescriptorTypeMask & (uint)GpuDescriptorTypeFlags::SAMPLER)
      return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;

    ASSERT(false, "unsupported descriptor type mask");
    return (D3D12_DESCRIPTOR_HEAP_TYPE)-1;
  }
//---------------------------------------------------------------------------//
  void CommandContextDX12::SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType, DescriptorHeapDX12* aDescriptorHeap)
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
  static void locMemcpySubresourceRows(const D3D12_MEMCPY_DEST* aDest, const D3D12_SUBRESOURCE_DATA* aSrc, size_t aRowStrideBytes, uint aNumRows, uint aNumSlices)
  {
    
  }
//---------------------------------------------------------------------------//
  void CommandContextDX12::UpdateSubresources(ID3D12Resource* aDestResource, ID3D12Resource* aStagingResource,
    uint aFirstSubresourceIndex, uint aNumSubresources, D3D12_SUBRESOURCE_DATA* someSubresourceDatas) const
  {
    D3D12_RESOURCE_DESC srcDesc = aStagingResource->GetDesc();
    D3D12_RESOURCE_DESC destDesc = aDestResource->GetDesc();
    
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT* destLayouts = static_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(alloca(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) * aNumSubresources));
    uint64* destRowSizesByte = static_cast<uint64*>(alloca(sizeof(uint64) * aNumSubresources));
    uint* destRowNums = static_cast<uint*>(alloca(sizeof(uint) * aNumSubresources));

    uint64 destTotalSizeBytes = 0u;
    RenderCore::GetPlatformDX12()->GetDevice()->GetCopyableFootprints(&destDesc, aFirstSubresourceIndex, aNumSubresources, 0u, destLayouts, destRowNums, destRowSizesByte, &destTotalSizeBytes);

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
  void CommandContextDX12::ApplyDescriptorHeaps()
  {
    ID3D12DescriptorHeap* heapsToBind[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    memset(heapsToBind, 0, sizeof(heapsToBind));
    uint numHeapsToBind = 0u;

    for (DescriptorHeapDX12* heap : myDynamicShaderVisibleHeaps)
      if (heap != nullptr)
        heapsToBind[numHeapsToBind++] = heap->GetHeap();

    if (numHeapsToBind > 0u)
      myCommandList->SetDescriptorHeaps(numHeapsToBind, heapsToBind);
  }
//---------------------------------------------------------------------------//
  void CommandContextDX12::ReleaseAllocator(uint64 aFenceVal)
  {
    if (myCommandAllocator != nullptr)
    {
      RenderCore::GetPlatformDX12()->ReleaseCommandAllocator(myCommandAllocator, aFenceVal);
      myCommandAllocator = nullptr;
    }
  }
  //---------------------------------------------------------------------------//
  void CommandContextDX12::ReleaseDynamicHeaps(uint64 aFenceVal)
  {
    for (DescriptorHeapDX12* heap : myDynamicShaderVisibleHeaps)
    {
      if (heap != nullptr)
        RenderCore::GetPlatformDX12()->ReleaseDynamicDescriptorHeap(heap, aFenceVal);
    }

    for (DescriptorHeapDX12* heap : myRetiredDescriptorHeaps)
    {
      RenderCore::GetPlatformDX12()->ReleaseDynamicDescriptorHeap(heap, aFenceVal);
    }

    myRetiredDescriptorHeaps.clear();
    memset(myDynamicShaderVisibleHeaps, 0, sizeof(myDynamicShaderVisibleHeaps));
  }
//---------------------------------------------------------------------------//
  DescriptorDX12 CommandContextDX12::CopyDescriptorsToDynamicHeapRange(const DescriptorDX12* someResources, uint aResourceCount)
  {
    ASSERT(aResourceCount > 0u);

    const DescriptorDX12& firstDescriptor = someResources[0];

    D3D12_DESCRIPTOR_HEAP_TYPE heapType = firstDescriptor.myHeapType;
    DescriptorHeapDX12* dynamicHeap = myDynamicShaderVisibleHeaps[heapType];

    RenderCore_PlatformDX12* platformDx12 = RenderCore::GetPlatformDX12();

    if (dynamicHeap == nullptr || dynamicHeap->GetNumFreeDescriptors() < aResourceCount)
    {
      dynamicHeap = platformDx12->AllocateDynamicDescriptorHeap(aResourceCount, heapType);
      SetDescriptorHeap(heapType, dynamicHeap);
    }

    uint startOffset = dynamicHeap->GetNumAllocatedDescriptors();
    for (uint i = 0u; i < aResourceCount; ++i)
    {
      DescriptorDX12 destDescriptor = dynamicHeap->AllocateDescriptor();
      platformDx12->GetDevice()->CopyDescriptorsSimple(1, destDescriptor.myCpuHandle, someResources[i].myCpuHandle, heapType);
    }

    return dynamicHeap->GetDescriptor(startOffset);
  }
//---------------------------------------------------------------------------//
  void CommandContextDX12::ClearRenderTarget(TextureView* aTextureView, const float* aColor)
  {
    const GpuResourceViewDataDX12& viewDataDx12 = aTextureView->myNativeData.To<GpuResourceViewDataDX12>();

    ASSERT(aTextureView->GetProperties().myIsRenderTarget);
    ASSERT(viewDataDx12.myType == GpuResourceViewDataDX12::RTV);

    // TODO: Move out to caller?
    TransitionResource(aTextureView->GetTexture(), GpuResourceState::RESOURCE_STATE_RENDER_TARGET);

    myCommandList->ClearRenderTargetView(viewDataDx12.myDescriptor.myCpuHandle, aColor, 0, nullptr);
  }
  //---------------------------------------------------------------------------//
  void CommandContextDX12::ClearDepthStencilTarget(TextureView* aTextureView, float aDepthClear, uint8 aStencilClear, uint someClearFlags)
  {
    const GpuResourceViewDataDX12& viewDataDx12 = aTextureView->myNativeData.To<GpuResourceViewDataDX12>();
    ASSERT(viewDataDx12.myType == GpuResourceViewDataDX12::DSV);

    // TODO: Move out to caller?
    TransitionResource(aTextureView->GetTexture(), GpuResourceState::RESOURCE_STATE_DEPTH_WRITE);

    D3D12_CLEAR_FLAGS clearFlags = (D3D12_CLEAR_FLAGS)0;
    if (someClearFlags & (uint)DepthStencilClearFlags::CLEAR_DEPTH)
      clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
    if (someClearFlags & (uint)DepthStencilClearFlags::CLEAR_STENCIL)
      clearFlags |= D3D12_CLEAR_FLAG_STENCIL;

    myCommandList->ClearDepthStencilView(viewDataDx12.myDescriptor.myCpuHandle, clearFlags, aDepthClear, aStencilClear, 0, nullptr);
  }
//---------------------------------------------------------------------------//
  void CommandContextDX12::CopyResource(GpuResource* aDestResource, GpuResource* aSrcResource)
  {
    locValidateUsageStatesCopy(aDestResource, aSrcResource);

    GpuResourceStorageDX12* destStorage = (GpuResourceStorageDX12*)aDestResource->myStorage.get();
    GpuResourceStorageDX12* srcStorage = (GpuResourceStorageDX12*)aSrcResource->myStorage.get();
    
    myCommandList->CopyResource(destStorage->myResource.Get(), srcStorage->myResource.Get());
  }
//---------------------------------------------------------------------------//
  void CommandContextDX12::CopyBufferRegion(const GpuBuffer* aDestBuffer, uint64 aDestOffset, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset, uint64 aSize)
  {
    ASSERT(aDestBuffer != aSrcBuffer, "Copying within the same buffer is not supported (same subresource)");
    ASSERT(aSize <= aDestBuffer->GetByteSize() - aDestOffset, "Invalid dst-region specified");
    ASSERT(aSize <= aSrcBuffer->GetByteSize() - aSrcOffset, "Invalid src-region specified");
    locValidateUsageStatesCopy(aDestBuffer, aSrcBuffer);

    ID3D12Resource* dstResource = ((GpuResourceStorageDX12*)aDestBuffer->myStorage.get())->myResource.Get();
    ID3D12Resource* srcResource = ((GpuResourceStorageDX12*)aSrcBuffer->myStorage.get())->myResource.Get();

    myCommandList->CopyBufferRegion(dstResource, aDestOffset, srcResource, aSrcOffset, aSize);
  }
//---------------------------------------------------------------------------//
  void CommandContextDX12::CopyTextureRegion(const Texture* aDestTexture, const TextureSubLocation& aDestSubLocation, const TextureRegion& aDestRegion, const Texture* aSrcTexture, const TextureSubLocation& aSrcSubLocation, const TextureRegion& aSrcRegion)
  {
    const TextureProperties& dstProps = aDestTexture->GetProperties();
    const TextureProperties& srcProps = aSrcTexture->GetProperties();

    // TODO: asserts and validation of regions agains texture-parameters
    
    ID3D12Resource* dstResource = ((GpuResourceStorageDX12*)aDestTexture->myStorage.get())->myResource.Get();
    ID3D12Resource* srcResource = ((GpuResourceStorageDX12*)aSrcTexture->myStorage.get())->myResource.Get();

    D3D12_TEXTURE_COPY_LOCATION dstLocation;
    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLocation.SubresourceIndex = aDestTexture->GetSubresourceIndex(aDestSubLocation);
    dstLocation.pResource = dstResource;

    D3D12_TEXTURE_COPY_LOCATION srcLocation;
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    srcLocation.SubresourceIndex = aSrcTexture->GetSubresourceIndex(aSrcSubLocation);
    srcLocation.pResource = srcResource;
    
    locValidateUsageStatesCopy(aDestTexture, dstLocation.SubresourceIndex, aSrcTexture, srcLocation.SubresourceIndex);

    D3D12_BOX srcBox;
    srcBox.left = aSrcRegion.myTexelPos.x;
    srcBox.right = aSrcRegion.myTexelPos.x + aSrcRegion.myTexelSize.x;
    srcBox.bottom = aSrcRegion.myTexelPos.y;
    srcBox.top = aSrcRegion.myTexelPos.y + aSrcRegion.myTexelSize.y;
    srcBox.front = aSrcRegion.myTexelPos.z;
    srcBox.back = aSrcRegion.myTexelPos.z + aSrcRegion.myTexelSize.z;

    myCommandList->CopyTextureRegion(&dstLocation, aDestRegion.myTexelPos.x, aDestRegion.myTexelPos.y, aDestRegion.myTexelPos.z, &srcLocation, &srcBox);
  }
//---------------------------------------------------------------------------//
  void CommandContextDX12::CopyTextureRegion(const Texture* aDestTexture, const TextureSubLocation& aDestSubLocation, const TextureRegion& aDestRegion, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset)
  {
    const TextureProperties& dstProps = aDestTexture->GetProperties();

    ID3D12Resource* dstResource = ((GpuResourceStorageDX12*)aDestTexture->myStorage.get())->myResource.Get();
    ID3D12Resource* srcResource = ((GpuResourceStorageDX12*)aSrcBuffer->myStorage.get())->myResource.Get();

    D3D12_TEXTURE_COPY_LOCATION dstLocation;
    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLocation.SubresourceIndex = aDestTexture->GetSubresourceIndex(aDestSubLocation);
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
    
    myCommandList->CopyTextureRegion(&dstLocation, aDestRegion.myTexelPos.x, aDestRegion.myTexelPos.y, aDestRegion.myTexelPos.z, &srcLocation, nullptr);
  }
//---------------------------------------------------------------------------//
  void CommandContextDX12::TransitionResourceList(GpuResource** someResources, GpuResourceState* someTransitionToStates, uint aNumResources)
  {
    D3D12_RESOURCE_BARRIER* barriers = static_cast<D3D12_RESOURCE_BARRIER*>(alloca(sizeof(D3D12_RESOURCE_BARRIER) * aNumResources));
    uint numBarriers = 0;
    for (uint i = 0; i < aNumResources; ++i)
    {
      GpuResource* resource = someResources[i];
      GpuResourceState destState = someTransitionToStates[i];
      
      if (resource->myUsageState == destState)
        continue;

      D3D12_RESOURCE_BARRIER& barrier = barriers[numBarriers++];

      GpuResourceStorageDX12* storage = (GpuResourceStorageDX12*)resource->myStorage.get();
      barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
      barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
      barrier.Transition.pResource = storage->myResource.Get();
      barrier.Transition.StateBefore = Adapter::toNativeType(resource->myUsageState);
      barrier.Transition.StateAfter = Adapter::toNativeType(destState);
      barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

      resource->myUsageState = destState;
    }

    if (numBarriers > 0u)
      myCommandList->ResourceBarrier(numBarriers, barriers);
  }
//---------------------------------------------------------------------------//
  void CommandContextDX12::Reset(uint64 aFenceVal)
  {
    CommandContext::Reset(aFenceVal);
    
    ReleaseDynamicHeaps(aFenceVal);
    ReleaseAllocator(aFenceVal);

    myCommandAllocator = RenderCore::GetPlatformDX12()->GetCommandAllocator(myCommandListType);
    ASSERT(myCommandAllocator != nullptr);
    
    CloseCommandList();
    CheckD3Dcall(myCommandList->Reset(myCommandAllocator, nullptr));
    myCommandListIsClosed = false;

    myRootSignature = nullptr;
    memset(myDynamicShaderVisibleHeaps, 0u, sizeof(myDynamicShaderVisibleHeaps));
  }
//---------------------------------------------------------------------------//
  D3D12_GRAPHICS_PIPELINE_STATE_DESC CommandContextDX12::GetNativePSOdesc(const GraphicsPipelineState& aState)
  {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    memset(&psoDesc, 0u, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

    // SHADER BYTECODES
    D3D12_SHADER_BYTECODE* shaderDescs[]{ &psoDesc.VS, &psoDesc.PS, &psoDesc.DS, &psoDesc.HS, &psoDesc.GS };
    ASSERT(ARRAY_LENGTH(shaderDescs) == (uint)ShaderStage::NUM_NO_COMPUTE);

    if (aState.myGpuProgramPipeline != nullptr)
    {
      for (uint i = 0u; i < (uint)ShaderStage::NUM_NO_COMPUTE; ++i)
      {
        if (nullptr == aState.myGpuProgramPipeline->myGpuPrograms[i])
          continue;

        const GpuProgramDX12* shaderDx12 = static_cast<const GpuProgramDX12*>(aState.myGpuProgramPipeline->myGpuPrograms[i].get());

        (*shaderDescs[i]) = shaderDx12->getNativeByteCode();
      }
    }

    // ROOT SIGNATURE
    const ShaderResourceInterfaceDX12* sriDx12 = static_cast<const ShaderResourceInterfaceDX12*>(aState.myGpuProgramPipeline->myResourceInterface);
    psoDesc.pRootSignature = sriDx12->myRootSignature.Get();

    // BLEND DESC
    D3D12_BLEND_DESC& blendDesc = psoDesc.BlendState;
    memset(&blendDesc, 0u, sizeof(D3D12_BLEND_DESC));
    blendDesc.AlphaToCoverageEnable = aState.myBlendState->myAlphaToCoverageEnabled;
    blendDesc.IndependentBlendEnable = aState.myBlendState->myBlendStatePerRT;
    uint rtCount = blendDesc.IndependentBlendEnable ? Constants::kMaxNumRenderTargets : 1u;
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

    if (aState.myGpuProgramPipeline != nullptr &&
      aState.myGpuProgramPipeline->myGpuPrograms[(uint)ShaderStage::VERTEX] != nullptr)
    {
      const GpuProgramDX12* vertexShader =
        static_cast<const GpuProgramDX12*>(aState.myGpuProgramPipeline->myGpuPrograms[(uint)ShaderStage::VERTEX].get());

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
      psoDesc.RTVFormats[i] = RenderCore_PlatformDX12::GetFormat(aState.myRTVformats[i]);
    }

    // DSV FORMAT
    psoDesc.DSVFormat = RenderCore_PlatformDX12::GetDepthStencilViewFormat(RenderCore_PlatformDX12::GetFormat(aState.myDSVformat));

    // NODE MASK
    psoDesc.NodeMask = 0u;

    return psoDesc;
  }
//---------------------------------------------------------------------------//
  D3D12_COMPUTE_PIPELINE_STATE_DESC CommandContextDX12::GetNativePSOdesc(const ComputePipelineState& aState)
  {
    D3D12_COMPUTE_PIPELINE_STATE_DESC desc;
    memset(&desc, 0u, sizeof(desc));

    if (aState.myGpuProgram != nullptr)
    {
      const GpuProgramDX12* gpuProgramDx12 =
        static_cast<const GpuProgramDX12*>(aState.myGpuProgram);

      desc.pRootSignature = gpuProgramDx12->GetRootSignature();
      desc.CS = gpuProgramDx12->getNativeByteCode();
    }

    desc.NodeMask = 0u;
    return desc;
  }
//---------------------------------------------------------------------------//
  void CommandContextDX12::BindBuffer(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someViewProperties, uint aRegisterIndex) const
  {
    ASSERT(myRootSignature != nullptr);
    
    GpuResourceStorageDX12* storage = (GpuResourceStorageDX12*)aBuffer->myStorage.get();

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
    
    switch (myCommandListType)
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
  void CommandContextDX12::BindResourceSet(const GpuResourceView** someResourceViews, uint aResourceCount, uint aRegisterIndex)
  {
    ASSERT(myRootSignature != nullptr);

    DescriptorDX12* dx12Descriptors = (DescriptorDX12*)alloca(sizeof(DescriptorDX12) * aResourceCount);
    for (uint i = 0; i < aResourceCount; ++i)
    {
      ASSERT(someResourceViews[i]->myNativeData.HasType<GpuResourceViewDataDX12>());
      const GpuResourceViewDataDX12& resourceViewData = someResourceViews[i]->myNativeData.To<GpuResourceViewDataDX12>();
      dx12Descriptors[i] = resourceViewData.myDescriptor;
    }

    const DescriptorDX12 dynamicRangeStartDescriptor = CopyDescriptorsToDynamicHeapRange(dx12Descriptors, aResourceCount);

    switch(myCommandListType)
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
  void CommandContextDX12::SetGpuProgramPipeline(const SharedPtr<GpuProgramPipeline>& aGpuProgramPipeline)
  {
    CommandContext::SetGpuProgramPipeline(aGpuProgramPipeline);

    const ShaderResourceInterfaceDX12* sriDx12 =
      static_cast<const ShaderResourceInterfaceDX12*>(aGpuProgramPipeline->myResourceInterface);

    if (myRootSignature != sriDx12->myRootSignature.Get())
    {
      myRootSignature = sriDx12->myRootSignature.Get();
      myCommandList->SetGraphicsRootSignature(myRootSignature);
    }
  }
  //---------------------------------------------------------------------------//
  void CommandContextDX12::SetVertexIndexBuffers(const GpuBuffer* aVertexBuffer, const GpuBuffer* anIndexBuffer, uint aVertexOffset, uint aNumVertices, uint anIndexOffset, uint aNumIndices)
  {
    // TODO: Check again if we need to apply all this stuff here or rather only before drawing
    ApplyViewportAndClipRect();
    ApplyRenderTargets();
    ApplyPipelineState();

    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    {
      GpuResourceStorageDX12* storage = (GpuResourceStorageDX12*)aVertexBuffer->myStorage.get();
      const GpuBufferProperties& bufferParams = aVertexBuffer->GetProperties();

      vertexBufferView.BufferLocation = storage->myResource->GetGPUVirtualAddress() + aVertexOffset * bufferParams.myElementSizeBytes;

      const uint64 byteSize = glm::min((uint64)aNumVertices, bufferParams.myNumElements) * bufferParams.myElementSizeBytes;
      ASSERT(byteSize <= UINT_MAX);

      vertexBufferView.SizeInBytes = static_cast<uint>(byteSize);
      vertexBufferView.StrideInBytes = static_cast<uint>(bufferParams.myElementSizeBytes);
    }

    D3D12_INDEX_BUFFER_VIEW indexBufferView;
    {
      GpuResourceStorageDX12* storage = (GpuResourceStorageDX12*)anIndexBuffer->myStorage.get();
      const GpuBufferProperties& bufferParams = anIndexBuffer->GetProperties();

      indexBufferView.BufferLocation = storage->myResource->GetGPUVirtualAddress() + anIndexOffset * bufferParams.myElementSizeBytes;
      indexBufferView.Format = bufferParams.myElementSizeBytes == 2u ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

      const uint64 byteSize = glm::min((uint64)aNumIndices, bufferParams.myNumElements) * bufferParams.myElementSizeBytes;
      ASSERT(byteSize <= UINT_MAX);

      indexBufferView.SizeInBytes = static_cast<uint>(byteSize);
    }

    myCommandList->IASetPrimitiveTopology(Adapter::ResolveTopology(myGraphicsPipelineState.myTopologyType));
    myCommandList->IASetVertexBuffers(0, 1, &vertexBufferView);
    myCommandList->IASetIndexBuffer(&indexBufferView);
  }
  //---------------------------------------------------------------------------//
  void CommandContextDX12::Render(uint aNumIndicesPerInstance, uint aNumInstances, uint anIndexOffset, uint aVertexOffset, uint anInstanceOffset)
  {
    ApplyViewportAndClipRect();
    ApplyRenderTargets();
    ApplyPipelineState();

    myCommandList->DrawIndexedInstanced(aNumIndicesPerInstance, aNumInstances, anIndexOffset, aVertexOffset, anInstanceOffset);
  }
  //---------------------------------------------------------------------------//
  void CommandContextDX12::RenderGeometry(const GeometryData* pGeometry)
  {
    const GpuBufferDX12* vertexBufferDx12 = static_cast<const GpuBufferDX12*>(pGeometry->getVertexBuffer());
    const GpuBufferDX12* indexBufferDx12 = static_cast<const GpuBufferDX12*>(pGeometry->getIndexBuffer());

    SetTopologyType(pGeometry->getGeometryVertexLayout().myTopology);
    SetVertexIndexBuffers(vertexBufferDx12, indexBufferDx12);
    Render(indexBufferDx12->GetProperties().myNumElements, 1, 0, 0, 0);
  }
  //---------------------------------------------------------------------------//
  void CommandContextDX12::ApplyViewportAndClipRect()
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
  void CommandContextDX12::ApplyRenderTargets()
  {
    if (!myRenderTargetsDirty)
      return;

    myRenderTargetsDirty = false;

    D3D12_CPU_DESCRIPTOR_HANDLE rtDescriptors[Constants::kMaxNumRenderTargets];
    GpuResource* rtResources[Constants::kMaxNumRenderTargets];
    uint numRtsToSet = 0u;

    for (uint i = 0u; i < Constants::kMaxNumRenderTargets; ++i)
    {
      const GpuResourceViewDataDX12& viewData = myRenderTargets[i]->myNativeData.To<GpuResourceViewDataDX12>();
      ASSERT(viewData.myType == GpuResourceViewDataDX12::RTV);

      rtResources[numRtsToSet] = myRenderTargets[i]->GetTexture();
      rtDescriptors[numRtsToSet] = viewData.myDescriptor.myCpuHandle;
      ++numRtsToSet;
    }

    for (uint i = 0u; i < numRtsToSet; ++i)
      TransitionResource(rtResources[i], GpuResourceState::RESOURCE_STATE_RENDER_TARGET);

    if (myDepthStencilTarget != nullptr)
    {
      const GpuResourceViewDataDX12& dsvViewData = myDepthStencilTarget->myNativeData.To<GpuResourceViewDataDX12>();
      ASSERT(dsvViewData.myType == GpuResourceViewDataDX12::DSV);
      TransitionResource(myDepthStencilTarget->GetTexture(), myDepthStencilTarget->GetProperties().myIsDepthReadOnly ? GpuResourceState::RESOURCE_STATE_DEPTH_READ : GpuResourceState::RESOURCE_STATE_DEPTH_WRITE);

      myCommandList->OMSetRenderTargets(numRtsToSet, rtDescriptors, false, &dsvViewData.myDescriptor.myCpuHandle);
    }
    else
    {
      myCommandList->OMSetRenderTargets(numRtsToSet, rtDescriptors, false, nullptr);
    }
  }
//---------------------------------------------------------------------------//
  void CommandContextDX12::ApplyPipelineState()
  {
    switch (myCommandListType)
    {
    case CommandListType::Graphics: 
      ApplyGraphicsPipelineState();
      break;
    case CommandListType::Compute: 
      ApplyComputePipelineState();
      break;
    case CommandListType::DMA: break;
    case CommandListType::NUM: break;
    default: break;
    }
  }
//---------------------------------------------------------------------------//
  void CommandContextDX12::ApplyGraphicsPipelineState()
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
  void CommandContextDX12::ApplyComputePipelineState()
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
  void CommandContextDX12::SetComputeProgram(const GpuProgram* aProgram)
  {
    CommandContext::SetComputeProgram(aProgram);

    const GpuProgramDX12* programDx12 = static_cast<const GpuProgramDX12*>(aProgram);

    if (myRootSignature != programDx12->GetRootSignature())
    {
      myRootSignature = programDx12->GetRootSignature();
      myCommandList->SetComputeRootSignature(myRootSignature);
    }
  }
//---------------------------------------------------------------------------//
  void CommandContextDX12::Dispatch(uint aThreadGroupCountX, uint aThreadGroupCountY, uint aThreadGroupCountZ)
  {
    ApplyPipelineState();
    myCommandList->Dispatch(aThreadGroupCountX, aThreadGroupCountY, aThreadGroupCountZ);
  }
//---------------------------------------------------------------------------//
  void CommandContextDX12::CloseCommandList()
  {
    if (!myCommandListIsClosed)
    {
      CheckD3Dcall(myCommandList->Close());
      myCommandListIsClosed = true;
    }
  }
//---------------------------------------------------------------------------//
}
