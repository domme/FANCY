#include "fancy_core_precompile.h"
#include "CommandListDX12.h"

#if FANCY_ENABLE_DX12

#include "Common/FancyCoreDefines.h"
#include "Common/TimeManager.h"

#include "Rendering/RenderCore.h"
#include "Rendering/BlendState.h"

#include "GpuBufferDX12.h"
#include "RenderCore_PlatformDX12.h"
#include "TextureDX12.h"
#include "AdapterDX12.h"
#include "ShaderDX12.h"
#include "ShaderPipelineDX12.h"
#include "GpuResourceDataDX12.h"
#include "GpuResourceViewDataDX12.h"
#include "GpuQueryHeapDX12.h"
#include "DebugUtilsDX12.h"
#include "RtPipelineStateDX12.h"

#include "WinPixEventRuntime/pix3.h"

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
    D3D12_RESOURCE_STATES locGetResourceStatesForContext(CommandListType aCommandListType)
    {
      switch (aCommandListType)
      {
        case CommandListType::Graphics: 
          return (D3D12_RESOURCE_STATES)UINT_MAX;
        case CommandListType::Compute:
          return D3D12_RESOURCE_STATE_COMMON | D3D12_RESOURCE_STATE_UNORDERED_ACCESS | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT | D3D12_RESOURCE_STATE_COPY_DEST | D3D12_RESOURCE_STATE_COPY_SOURCE | D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        case CommandListType::DMA: 
          return D3D12_RESOURCE_STATE_COMMON | D3D12_RESOURCE_STATE_COPY_DEST | D3D12_RESOURCE_STATE_COPY_SOURCE;
        default: return (D3D12_RESOURCE_STATES) 0u;
      }
    }
//---------------------------------------------------------------------------//
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  CommandListDX12::CommandListDX12(CommandListType aCommandListType)
    : CommandList(aCommandListType)
    , myCommandList(nullptr)
    , myCommandAllocator(nullptr)
    , myResourceStateMask(locGetResourceStatesForContext(aCommandListType))
  {
    myCommandAllocator = RenderCore::GetPlatformDX12()->GetCommandAllocator(myCommandListType);

    D3D12_COMMAND_LIST_TYPE nativeCmdListType = locResolveCommandListType(aCommandListType);

    ASSERT_HRESULT(
      RenderCore::GetPlatformDX12()->GetDevice()->CreateCommandList(0, nativeCmdListType,
        myCommandAllocator, nullptr, IID_PPV_ARGS(&myCommandList))
    );

    PrepareForRecord(false);
  }
//---------------------------------------------------------------------------//
  CommandListDX12::~CommandListDX12()
  {
    CommandListDX12::PostExecute(0ull);
    
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
  void CommandListDX12::PrepareForRecord(bool aResetCommandList)
  {
    RenderCore_PlatformDX12* platformDx12 = RenderCore::GetPlatformDX12();

    if (aResetCommandList)
      ASSERT_HRESULT(myCommandList->Reset(myCommandAllocator, nullptr));

    myTopologyDirty = true;
    myPendingBarriers.clear();
    myLocalHazardData.clear();

    const ShaderVisibleDescriptorHeapDX12* shaderVisibleHeap = platformDx12->GetShaderVisibleDescriptorHeap();

    // We only use one shader-visible descriptor heap per type, so just bind them up-front
    ID3D12DescriptorHeap* shaderVisibleHeaps[] = {
      shaderVisibleHeap->GetResourceHeap(),
      shaderVisibleHeap->GetSamplerHeap()
    };
    myCommandList->SetDescriptorHeaps(ARRAY_LENGTH(shaderVisibleHeaps), shaderVisibleHeaps);

    // Set the root signature up front since we only use one
    const RootSignatureDX12* rootSignature = platformDx12->GetRootSignature();
    if (myCommandListType == CommandListType::Graphics || myCommandListType == CommandListType::Compute)
    {
      myCommandList->SetComputeRootSignature(rootSignature->GetRootSignature());
      myCommandList->SetComputeRootDescriptorTable(rootSignature->myRootParamIndex_GlobalResources, shaderVisibleHeap->GetResourceHeapStart());
      myCommandList->SetComputeRootDescriptorTable(rootSignature->myRootParamIndex_GlobalSamplers, shaderVisibleHeap->GetSamplerHeapStart());
    }

    if (myCommandListType == CommandListType::Graphics)
    {
      myCommandList->SetGraphicsRootSignature(rootSignature->GetRootSignature());
      myCommandList->SetGraphicsRootDescriptorTable(rootSignature->myRootParamIndex_GlobalResources, shaderVisibleHeap->GetResourceHeapStart());
      myCommandList->SetGraphicsRootDescriptorTable(rootSignature->myRootParamIndex_GlobalSamplers, shaderVisibleHeap->GetSamplerHeapStart());
    }
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::UpdateSubresources(ID3D12Resource* aDstResource, ID3D12Resource* aStagingResource,
    uint aFirstSubresourceIndex, uint aNumSubresources, D3D12_SUBRESOURCE_DATA* someSubresourceDatas)
  {
    D3D12_RESOURCE_DESC srcDesc = aStagingResource->GetDesc();
    D3D12_RESOURCE_DESC destDesc = aDstResource->GetDesc();
    
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
      myCommandList->CopyBufferRegion(aDstResource, 0, aStagingResource, destLayouts[0].Offset, destLayouts[0].Footprint.Width);
    }
    else
    {
      for (uint i = 0u; i < aNumSubresources; ++i)
      {
        D3D12_TEXTURE_COPY_LOCATION destCopyLocation;
        destCopyLocation.pResource = aDstResource;
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
  void CommandListDX12::ClearRenderTarget(TextureView* aTextureView, const float* aColor)
  {
    const GpuResourceViewDataDX12& viewDataDx12 = eastl::any_cast<const GpuResourceViewDataDX12&>(aTextureView->myNativeData);

    ASSERT(aTextureView->GetProperties().myIsRenderTarget);
    ASSERT(aTextureView->myType == GpuResourceViewType::RTV);

    TrackResourceTransition(aTextureView->GetTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    FlushBarriers();

    myCommandList->ClearRenderTargetView(viewDataDx12.myDescriptor.myCpuHandle, aColor, 0, nullptr);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::ClearDepthStencilTarget(TextureView* aTextureView, float aDepthClear, uint8 aStencilClear, uint someClearFlags)
  {
    const GpuResourceViewDataDX12& viewDataDx12 = eastl::any_cast<const GpuResourceViewDataDX12&>(aTextureView->myNativeData);
    ASSERT(aTextureView->myType == GpuResourceViewType::DSV);

    const DataFormat format = aTextureView->GetTexture()->GetProperties().myFormat;
    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(format);
    ASSERT(formatInfo.myIsDepthStencil);

    const bool clearDepth = someClearFlags & (uint)DepthStencilClearFlags::CLEAR_DEPTH;
    const bool clearStencil = someClearFlags & (uint)DepthStencilClearFlags::CLEAR_STENCIL;

    const SubresourceRange& subresources = aTextureView->GetSubresourceRange();

    if (clearDepth && !clearStencil)
    {
      ASSERT(subresources.myFirstPlane == 0, "The texture view doesn't cover the depth plane");
    }
    else if (clearStencil && !clearDepth)
    {
      ASSERT(formatInfo.myNumPlanes == 2);
      ASSERT(subresources.myFirstPlane + subresources.myNumPlanes >= 2, "The texture view doesn't cover the stencil plane");
    }
    else
    {
      ASSERT(formatInfo.myNumPlanes == 2);
      ASSERT(subresources.myFirstPlane == 0, "The texture view doesn't cover the depth plane");
      ASSERT(subresources.myFirstPlane + subresources.myNumPlanes >= 2, "The texture view doesn't cover the stencil plane");
    }

    TrackResourceTransition(aTextureView->GetTexture(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
    FlushBarriers();
    
    D3D12_CLEAR_FLAGS clearFlags = (D3D12_CLEAR_FLAGS)0;
    if (someClearFlags & (uint)DepthStencilClearFlags::CLEAR_DEPTH)
      clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
    if (someClearFlags & (uint)DepthStencilClearFlags::CLEAR_STENCIL)
      clearFlags |= D3D12_CLEAR_FLAG_STENCIL;

    myCommandList->ClearDepthStencilView(viewDataDx12.myDescriptor.myCpuHandle, clearFlags, aDepthClear, aStencilClear, 0, nullptr);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::CopyResource(GpuResource* aDstResource, GpuResource* aSrcResource)
  {
    TrackResourceTransition(aSrcResource, D3D12_RESOURCE_STATE_COPY_SOURCE);
    TrackResourceTransition(aDstResource, D3D12_RESOURCE_STATE_COPY_DEST);

    FlushBarriers();

    GpuResourceDataDX12* destData = aDstResource->GetDX12Data();
    GpuResourceDataDX12* srcData = aSrcResource->GetDX12Data();

    myCommandList->CopyResource(destData->myResource.Get(), srcData->myResource.Get());
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::CopyBuffer(const GpuBuffer* aDstBuffer, uint64 aDstOffset, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset, uint64 aSize)
  {
    ASSERT(aDstBuffer != aSrcBuffer, "Copying within the same buffer is not supported (same subresource)");

#if FANCY_RENDERER_USE_VALIDATION
    ValidateBufferCopy(aDstBuffer->GetProperties(), aDstOffset, aSrcBuffer->GetProperties(), aSrcOffset, aSize);
#endif

    TrackResourceTransition(aSrcBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);
    TrackResourceTransition(aDstBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

    FlushBarriers();

    ID3D12Resource* dstResource = static_cast<const GpuBufferDX12*>(aDstBuffer)->GetData()->myResource.Get();
    ID3D12Resource* srcResource = static_cast<const GpuBufferDX12*>(aSrcBuffer)->GetData()->myResource.Get();

    myCommandList->CopyBufferRegion(dstResource, aDstOffset, srcResource, aSrcOffset, aSize);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::CopyTextureToBuffer(const GpuBuffer* aDstBuffer, uint64 aDstOffset, const Texture* aSrcTexture, const SubresourceLocation& aSrcSubresource, const TextureRegion& aSrcRegion)
  {
#if FANCY_RENDERER_USE_VALIDATION
    ValidateTextureToBufferCopy(aDstBuffer->GetProperties(), aDstOffset, aSrcTexture->GetProperties(), aSrcSubresource, aSrcRegion);
#endif

    ID3D12Resource* bufferResourceDX12 = static_cast<const GpuBufferDX12*>(aDstBuffer)->GetData()->myResource.Get();
    ID3D12Resource* textureResourceDX12 = static_cast<const TextureDX12*>(aSrcTexture)->GetData()->myResource.Get();

    const uint16 textureSubresourceIndex = static_cast<uint16>(aSrcTexture->GetSubresourceIndex(aSrcSubresource));

    TrackSubresourceTransition(aSrcTexture, SubresourceRange(aSrcSubresource), D3D12_RESOURCE_STATE_COPY_SOURCE);
    TrackResourceTransition(aDstBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

    D3D12_TEXTURE_COPY_LOCATION srcLocation;
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    srcLocation.SubresourceIndex = textureSubresourceIndex;
    srcLocation.pResource = textureResourceDX12;

    const DataFormat format = aSrcTexture->GetProperties().myFormat;
    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(format);
    const DXGI_FORMAT formatDx12 = RenderCore_PlatformDX12::ResolveFormat(format);

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
    footprint.Offset = 0;
    footprint.Footprint.Format = RenderCore_PlatformDX12::GetCopyableFormat(formatDx12, aSrcSubresource.myPlaneIndex);
    footprint.Footprint.Width = aSrcRegion.mySize.x;
    footprint.Footprint.RowPitch = (uint) MathUtil::Align((uint64) BITS_TO_BYTES(aSrcRegion.mySize.x * formatInfo.myCopyableBitsPerPixelPerPlane[aSrcSubresource.myPlaneIndex]), (uint64) RenderCore::GetPlatformCaps().myTextureRowAlignment);
    footprint.Footprint.Height = aSrcRegion.mySize.y;
    footprint.Footprint.Depth = aSrcRegion.mySize.z;

    D3D12_TEXTURE_COPY_LOCATION destLocation;
    destLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    destLocation.pResource = bufferResourceDX12;
    destLocation.PlacedFootprint.Footprint = footprint.Footprint;
    destLocation.PlacedFootprint.Offset = aDstOffset;

    FlushBarriers();

    D3D12_BOX srcBox;
    srcBox.left = aSrcRegion.myPos.x;
    srcBox.right = aSrcRegion.myPos.x + aSrcRegion.mySize.x;
    srcBox.top = aSrcRegion.myPos.y;
    srcBox.bottom = aSrcRegion.myPos.y + aSrcRegion.mySize.y;
    srcBox.front = aSrcRegion.myPos.z;
    srcBox.back = aSrcRegion.myPos.z + aSrcRegion.mySize.z;
    myCommandList->CopyTextureRegion(&destLocation, 0, 0, 0, &srcLocation, &srcBox);
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::CopyTexture(const Texture* aDstTexture, const SubresourceLocation& aDstSubresource, const TextureRegion& aDstRegion, 
    const Texture* aSrcTexture, const SubresourceLocation& aSrcSubresource, const TextureRegion& aSrcRegion)
  {
#if FANCY_RENDERER_USE_VALIDATION
    ValidateTextureCopy(aDstTexture->GetProperties(), aDstSubresource, aDstRegion, aSrcTexture->GetProperties(), aSrcSubresource, aSrcRegion);
#endif
    
    ID3D12Resource* dstResource = static_cast<const TextureDX12*>(aDstTexture)->GetData()->myResource.Get();
    ID3D12Resource* srcResource = static_cast<const TextureDX12*>(aSrcTexture)->GetData()->myResource.Get();

    const uint16 destSubResourceIndex = static_cast<uint16>(aDstTexture->GetSubresourceIndex(aDstSubresource));
    const uint16 srcSubResourceIndex = static_cast<uint16>(aSrcTexture->GetSubresourceIndex(aSrcSubresource));

    TrackSubresourceTransition(aSrcTexture, SubresourceRange(aSrcSubresource), D3D12_RESOURCE_STATE_COPY_SOURCE);
    TrackSubresourceTransition(aDstTexture, SubresourceRange(aDstSubresource), D3D12_RESOURCE_STATE_COPY_DEST);

    D3D12_TEXTURE_COPY_LOCATION dstLocation;
    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLocation.SubresourceIndex = destSubResourceIndex;
    dstLocation.pResource = dstResource;

    D3D12_TEXTURE_COPY_LOCATION srcLocation;
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    srcLocation.SubresourceIndex = srcSubResourceIndex;
    srcLocation.pResource = srcResource;

    FlushBarriers();

    D3D12_BOX srcBox;
    srcBox.left = aSrcRegion.myPos.x;
    srcBox.right = aSrcRegion.myPos.x + aSrcRegion.mySize.x;
    srcBox.top = aSrcRegion.myPos.y;
    srcBox.bottom = aSrcRegion.myPos.y + aSrcRegion.mySize.y;
    srcBox.front = aSrcRegion.myPos.z;
    srcBox.back = aSrcRegion.myPos.z + aSrcRegion.mySize.z;
    myCommandList->CopyTextureRegion(&dstLocation, aDstRegion.myPos.x, aDstRegion.myPos.y, aDstRegion.myPos.z, &srcLocation, &srcBox);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::CopyBufferToTexture(const Texture* aDstTexture, const SubresourceLocation& aDstSubresource, const TextureRegion& aDstRegion, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset)
  {
#if FANCY_RENDERER_USE_VALIDATION
    ValidateBufferToTextureCopy(aDstTexture->GetProperties(), aDstSubresource, aDstRegion, aSrcBuffer->GetProperties(), aSrcOffset);
#endif

    ID3D12Resource* dstResource = static_cast<const TextureDX12*>(aDstTexture)->GetData()->myResource.Get();
    ID3D12Resource* srcResource = static_cast<const GpuBufferDX12*>(aSrcBuffer)->GetData()->myResource.Get();

    const uint16 destSubResourceIndex = static_cast<uint16>(aDstTexture->GetSubresourceIndex(aDstSubresource));
    const uint16 srcSubResourceIndex = 0;

    TrackResourceTransition(aSrcBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);
    TrackSubresourceTransition(aDstTexture, SubresourceRange(aDstSubresource), D3D12_RESOURCE_STATE_COPY_DEST);
    
    D3D12_TEXTURE_COPY_LOCATION dstLocation;
    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLocation.SubresourceIndex = destSubResourceIndex;
    dstLocation.pResource = dstResource;

    const DataFormat format = aDstTexture->GetProperties().myFormat;
    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(format);
    const DXGI_FORMAT formatDx12 = RenderCore_PlatformDX12::ResolveFormat(format);

    uint64 rowPitch;
    uint heightBlocksOrPixel;
    TextureData::ComputeRowPitchSizeAndBlockHeight(format, aDstRegion.mySize.x, aDstRegion.mySize.y, rowPitch, heightBlocksOrPixel, aDstSubresource.myPlaneIndex);

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
    footprint.Offset = 0u;
    footprint.Footprint.Format = RenderCore_PlatformDX12::GetCopyableFormat(formatDx12, aDstSubresource.myPlaneIndex);
    footprint.Footprint.Width = aDstRegion.mySize.x;
    footprint.Footprint.RowPitch = (uint) MathUtil::Align(rowPitch, RenderCore::GetPlatformCaps().myTextureRowAlignment);
    footprint.Footprint.Height = aDstRegion.mySize.y;
    footprint.Footprint.Depth = aDstRegion.mySize.z;

    D3D12_TEXTURE_COPY_LOCATION srcLocation;
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    srcLocation.pResource = srcResource;
    srcLocation.PlacedFootprint.Footprint = footprint.Footprint;
    srcLocation.PlacedFootprint.Offset = aSrcOffset;

    FlushBarriers();

    myCommandList->CopyTextureRegion(&dstLocation, aDstRegion.myPos.x, aDstRegion.myPos.y, aDstRegion.myPos.z, &srcLocation, nullptr);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::UpdateTextureData(const Texture* aDstTexture, const SubresourceRange& aSubresourceRange, const TextureSubData* someDatas, uint aNumDatas)
  {
    const uint numSubresources = aSubresourceRange.GetNumSubresources();
    ASSERT(aNumDatas == numSubresources);
    
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT* footprints = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)alloca(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) * numSubresources);
    uint* rowNums = (uint*)alloca(sizeof(uint) * numSubresources);
    uint64* rowSizes = (uint64*)alloca(sizeof(uint64) * numSubresources);
    uint64 totalSize = static_cast<const TextureDX12*>(aDstTexture)->GetCopyableFootprints(aSubresourceRange, footprints, rowNums, rowSizes);

    uint64 uploadBufferOffset;
    const GpuBuffer* uploadBuffer = GetBuffer(uploadBufferOffset, GpuBufferUsage::STAGING_UPLOAD, nullptr, totalSize);
    ASSERT(uploadBuffer != nullptr);

    uint8* uploadBufferData = (uint8*)uploadBuffer->Map(GpuResourceMapMode::WRITE_UNSYNCHRONIZED, uploadBufferOffset, totalSize);

    for (uint i = 0; i < aNumDatas; ++i)
    {
      const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& footprint = footprints[i];
      uint numRows = rowNums[i];
      
      const TextureSubData& srcData = someDatas[i];
      ASSERT(rowSizes[i] == srcData.myRowSizeBytes);

      const uint64 alignedSliceSize = footprint.Footprint.RowPitch * numRows;

      uint8* dstSubresourceData = uploadBufferData + footprint.Offset;
      const uint8* srcSubresourceData = srcData.myData;
      for (uint iSlice = 0; iSlice < footprint.Footprint.Depth; ++iSlice)
      {
        uint8* dstSliceData = dstSubresourceData + iSlice * alignedSliceSize;
        const uint8* srcSliceData = srcSubresourceData + iSlice * srcData.mySliceSizeBytes;

        for (uint iRow = 0; iRow < numRows; ++iRow)
        {
          uint8* dstRowData = dstSliceData + iRow * footprint.Footprint.RowPitch;
          const uint8* srcRowData = srcSliceData + iRow * srcData.myRowSizeBytes;

          memcpy(dstRowData, srcRowData, srcData.myRowSizeBytes);
        }
      }
    }
    uploadBuffer->Unmap(GpuResourceMapMode::WRITE_UNSYNCHRONIZED, uploadBufferOffset, totalSize);

    TrackResourceTransition(aDstTexture, D3D12_RESOURCE_STATE_COPY_DEST);

    int i = 0;
    for (SubresourceIterator subIter = aSubresourceRange.Begin(), e = aSubresourceRange.End(); subIter != e; ++subIter)
    {
      const SubresourceLocation dstLocation = *subIter;
      CommandList::CopyBufferToTexture(aDstTexture, dstLocation, uploadBuffer, uploadBufferOffset + footprints[i++].Offset);
    }
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::PostExecute(uint64 aFenceVal)
  {
    CommandList::PostExecute(aFenceVal);

    if (myCommandAllocator != nullptr)
      RenderCore::GetPlatformDX12()->ReleaseCommandAllocator(myCommandAllocator, aFenceVal);
    myCommandAllocator = nullptr;

    myLocalHazardData.clear();
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::ResetAndOpen()
  {
    CommandList::ResetAndOpen();

    RenderCore_PlatformDX12* platformDx12 = RenderCore::GetPlatformDX12();
    myCommandAllocator = platformDx12->GetCommandAllocator(myCommandListType);
    ASSERT(myCommandAllocator != nullptr);

    PrepareForRecord(true);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::FlushBarriers()
  {
    if (!myPendingBarriers.empty())
    {
      myCommandList->ResourceBarrier((uint)myPendingBarriers.size(), myPendingBarriers.data());
      myPendingBarriers.clear();
    }
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::BindLocalBuffer(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someViewProperties, uint aRegisterIndex)
  {
    const RootSignatureDX12* rootSignature = RenderCore::GetPlatformDX12()->GetRootSignature();
    const GpuBufferDX12* bufferDx12 = static_cast<const GpuBufferDX12*>(aBuffer);
    const uint64 bufferViewGpuAddress = bufferDx12->GetDeviceAddress() + someViewProperties.myOffset;

    if (someViewProperties.myIsShaderWritable)
    {
      ASSERT(aRegisterIndex < rootSignature->myNumLocalRWBuffers);
      ASSERT(someViewProperties.myIsRaw || someViewProperties.myIsStructured, "D3D12 only supports raw or structured buffer SRVs/UAVs as root descriptor");
      TrackResourceTransition(aBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

      if (aRegisterIndex >= myLocalRWBuffersToBind.size())
        myLocalRWBuffersToBind.resize(aRegisterIndex + 1, UINT64_MAX);

      myLocalRWBuffersToBind[aRegisterIndex] = bufferViewGpuAddress;
    }
    else if (someViewProperties.myIsConstantBuffer)
    {
      ASSERT(aRegisterIndex < rootSignature->myNumLocalCBuffers);
      TrackResourceTransition(aBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

      if (aRegisterIndex >= myLocalCBuffersToBind.size())
        myLocalCBuffersToBind.resize(aRegisterIndex + 1, UINT64_MAX);

      myLocalCBuffersToBind[aRegisterIndex] = bufferViewGpuAddress;
    }
    else
    {
      ASSERT(aRegisterIndex < rootSignature->myNumLocalBuffers);
      ASSERT(someViewProperties.myIsRaw || someViewProperties.myIsStructured, "D3D12 only supports raw or structured buffer SRVs/UAVs as root descriptor");
      TrackResourceTransition(aBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

      if (aRegisterIndex >= myLocalBuffersToBind.size())
        myLocalBuffersToBind.resize(aRegisterIndex + 1, UINT64_MAX);

      myLocalBuffersToBind[aRegisterIndex] = bufferViewGpuAddress;
    }
  }
//---------------------------------------------------------------------------//
  GpuQuery CommandListDX12::BeginQuery(GpuQueryType aType)
  {
    ASSERT(aType != GpuQueryType::TIMESTAMP, "Timestamp-queries should be used with InsertTimestamp");

    const GpuQuery query = AllocateQuery(aType);
    GpuQueryHeap* heap = RenderCore::GetQueryHeap(aType);

    const GpuQueryHeapDX12* queryHeapDx12 = static_cast<const GpuQueryHeapDX12*>(heap);
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

    const GpuQueryHeapDX12* queryHeapDx12 = static_cast<const GpuQueryHeapDX12*>(heap);
    const D3D12_QUERY_TYPE queryTypeDx12 = Adapter::ResolveQueryType(queryType);

    myCommandList->EndQuery(queryHeapDx12->myHeap.Get(), queryTypeDx12, aQuery.myIndexInHeap);
  }
//---------------------------------------------------------------------------//
  GpuQuery CommandListDX12::InsertTimestamp()
  {
    const GpuQuery query = AllocateQuery(GpuQueryType::TIMESTAMP);
    query.myIsOpen = false;

    GpuQueryHeap* heap = RenderCore::GetQueryHeap(GpuQueryType::TIMESTAMP);
    const GpuQueryHeapDX12* queryHeapDx12 = static_cast<const GpuQueryHeapDX12*>(heap);

    myCommandList->EndQuery(queryHeapDx12->myHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, query.myIndexInHeap);
    return query;
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::CopyQueryDataToBuffer(const GpuQueryHeap* aQueryHeap, const GpuBuffer* aBuffer, uint aFirstQueryIndex, uint aNumQueries, uint64 aBufferOffset)
  {
    const GpuQueryHeapDX12* queryHeapDx12 = static_cast<const GpuQueryHeapDX12*>(aQueryHeap);
    const GpuBufferDX12* bufferDx12 = static_cast<const GpuBufferDX12*>(aBuffer);

    TrackResourceTransition(aBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
    FlushBarriers();

    myCommandList->ResolveQueryData(
      queryHeapDx12->myHeap.Get(),
      Adapter::ResolveQueryType(aQueryHeap->myType),
      aFirstQueryIndex,
      aNumQueries,
      bufferDx12->GetData()->myResource.Get(), 
      aBufferOffset);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::BeginMarkerRegion(const char* aName, uint aColor)
  {
    CommandList::BeginMarkerRegion(aName, aColor);
    PIXBeginEvent(myCommandList, aColor, aName);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::EndMarkerRegion()
  {
    CommandList::EndMarkerRegion();
    PIXEndEvent(myCommandList);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::TransitionResource(const GpuResource* aResource, const SubresourceRange& aSubresourceRange, ResourceTransition aTransition, uint /* someUsageFlags = 0u*/)
  {
    D3D12_RESOURCE_STATES newStates = (D3D12_RESOURCE_STATES) 0;
    bool toSharedRead = false;

    const GpuResourceHazardDataDX12& hazardData = aResource->GetDX12Data()->myHazardData;

    switch (aTransition)
    {
      case ResourceTransition::TO_SHARED_CONTEXT_READ: 
        newStates = (D3D12_RESOURCE_STATES) hazardData.myReadStates;
        toSharedRead = true;
        break;
      default: ASSERT(false); 
    }

    TrackSubresourceTransition(aResource, aSubresourceRange, newStates, toSharedRead);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::PrepareResourceShaderAccess(const GpuResource* aResource, const SubresourceRange& aSubresourceRange,  ShaderResourceAccess aTransition)
  {
    switch(aTransition)
    {
      case SHADER_RESOURCE_ACCESS_SRV:
        TrackSubresourceTransition(aResource, aSubresourceRange, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        break;
      case SHADER_RESOURCE_ACCESS_RTAS:
        TrackSubresourceTransition(aResource, aSubresourceRange, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
        break;
      case SHADER_RESOURCE_ACCESS_UAV:
        TrackSubresourceTransition(aResource, aSubresourceRange, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        break;
      default: ASSERT(false, "Missing implementation!");
    }
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

      AddBarrier(barrier);
    }
    else
    {
      D3D12_RESOURCE_BARRIER* barriers = (D3D12_RESOURCE_BARRIER*) alloca(sizeof(D3D12_RESOURCE_BARRIER) * aNumResources);
      for (uint iRes = 0u; iRes < aNumResources; ++iRes)
      {
        const GpuResource* resource = someResources[iRes];
        ID3D12Resource* resourceDx12 = resource->GetDX12Data()->myResource.Get();

        D3D12_RESOURCE_BARRIER& barrier = barriers[iRes];
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrier.UAV.pResource = resourceDx12;

        AddBarrier(barrier);
      }
    }
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::BindVertexBuffers(const GpuBuffer** someBuffers, uint64* someOffsets, uint64* someSizes, uint aNumBuffers, const VertexInputLayout* anInputLayout /*= nullptr*/)
  {
    const Shader* vertexShader = myGraphicsPipelineState.myShaderPipeline ? myGraphicsPipelineState.myShaderPipeline->GetShader(ShaderStage::SHADERSTAGE_VERTEX) : nullptr;
    const VertexInputLayout* shaderInputLayout = vertexShader ? vertexShader->myDefaultVertexInputLayout.get() : nullptr;
    const VertexInputLayout* inputLayout = anInputLayout ? anInputLayout : shaderInputLayout;

    ASSERT(inputLayout && inputLayout->myProperties.myBufferBindings.size() == aNumBuffers);

    const bool isDefaultLayout = inputLayout == shaderInputLayout;
    const bool pipelineStateIsDefault = myGraphicsPipelineState.myVertexInputLayout == nullptr;
    if (myGraphicsPipelineState.myVertexInputLayout != inputLayout && !(isDefaultLayout && pipelineStateIsDefault))
    {
      myGraphicsPipelineState.myVertexInputLayout = inputLayout;
      myGraphicsPipelineState.myIsDirty = true;
    }

    eastl::fixed_vector<D3D12_VERTEX_BUFFER_VIEW, 4> vertexBufferViews;
    for (uint i = 0u; i < aNumBuffers; ++i)
    {
      const GpuBuffer* buffer = someBuffers[i];
      TrackResourceTransition(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

      GpuResourceDataDX12* resourceDataDx12 = buffer->GetDX12Data();
      const uint64 resourceStartAddress = resourceDataDx12->myResource->GetGPUVirtualAddress();

      D3D12_VERTEX_BUFFER_VIEW& vertexBufferView = vertexBufferViews.push_back();
      vertexBufferView.BufferLocation = resourceStartAddress + someOffsets[i];
      ASSERT(someSizes[i] <= UINT_MAX);
      vertexBufferView.SizeInBytes = static_cast<uint>(someSizes[i]);

      ASSERT(inputLayout->myProperties.myBufferBindings[i].myStride <= vertexBufferView.SizeInBytes);
      vertexBufferView.StrideInBytes = inputLayout->myProperties.myBufferBindings[i].myStride;
    }

    myCommandList->IASetVertexBuffers(0, (uint) vertexBufferViews.size(), vertexBufferViews.data());
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

    TrackResourceTransition(aBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
    
    myCommandList->IASetIndexBuffer(&indexBufferView);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::DrawInstanced(uint aNumVerticesPerInstance, uint aNumInstances, uint aBaseVertex, uint aStartInstance)
  {
    CommandList::DrawInstanced(aNumVerticesPerInstance, aNumInstances, aBaseVertex, aStartInstance);
  
    ApplyViewportAndClipRect();
    ApplyRenderTargets();
    ApplyTopologyType();
    ApplyGraphicsPipelineState();
    FlushBarriers();
    ApplyResourceBindings();

    myCommandList->DrawInstanced(aNumVerticesPerInstance, aNumInstances, aBaseVertex, aStartInstance);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::DrawIndexedInstanced(uint aNumIndicesPerInstance, uint aNumInstances, uint aStartIndex, uint aBaseVertex, uint aStartInstance)
  {
    CommandList::DrawIndexedInstanced(aNumIndicesPerInstance, aNumInstances, aStartIndex, aBaseVertex, aStartInstance);

    ApplyViewportAndClipRect();
    ApplyRenderTargets();
    ApplyTopologyType();
    ApplyGraphicsPipelineState();
    FlushBarriers();
    ApplyResourceBindings();

    myCommandList->DrawIndexedInstanced(aNumIndicesPerInstance, aNumInstances, aStartIndex, aBaseVertex, aStartInstance);
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
    const uint numRtsToSet = myGraphicsPipelineState.myNumRenderTargets;
    D3D12_CPU_DESCRIPTOR_HANDLE rtDescriptors[RenderConstants::kMaxNumRenderTargets];

    for (uint i = 0u; i < numRtsToSet; ++i)
    {
      ASSERT(myRenderTargets[i] != nullptr);

      const GpuResourceViewDataDX12& viewData = eastl::any_cast<const GpuResourceViewDataDX12&>(myRenderTargets[i]->myNativeData);
      ASSERT(myRenderTargets[i]->myType == GpuResourceViewType::RTV);

      TrackSubresourceTransition(myRenderTargets[i]->GetResource(), myRenderTargets[i]->GetSubresourceRange(), D3D12_RESOURCE_STATE_RENDER_TARGET);

      rtDescriptors[i] = viewData.myDescriptor.myCpuHandle;
    }
    
    if (myDepthStencilTarget != nullptr)
    {
      const GpuResourceViewDataDX12& dsvViewData = eastl::any_cast<const GpuResourceViewDataDX12&>(myDepthStencilTarget->myNativeData);
      ASSERT(myDepthStencilTarget->myType == GpuResourceViewType::DSV);

      const D3D12_RESOURCE_STATES depthState = myDepthStencilTarget->GetProperties().myIsDepthReadOnly ? D3D12_RESOURCE_STATE_DEPTH_READ : D3D12_RESOURCE_STATE_DEPTH_WRITE;
      TrackSubresourceTransition(myDepthStencilTarget->GetResource(), myDepthStencilTarget->GetSubresourceRange(), depthState);

      if (myRenderTargetsDirty)
        myCommandList->OMSetRenderTargets(numRtsToSet, rtDescriptors, false, &dsvViewData.myDescriptor.myCpuHandle);
    }
    else
    {
      if (myRenderTargetsDirty)
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
  void CommandListDX12::ApplyResourceBindings()
  {
    RenderCore_PlatformDX12* platformDx12 = RenderCore::GetPlatformDX12();
    const RootSignatureDX12* rootSignature = platformDx12->GetRootSignature();
    
    if (myLocalBuffersToBind.empty() && myLocalRWBuffersToBind.empty() && myLocalCBuffersToBind.empty())
      return;

    uint rootParamIdx = rootSignature->myRootParamIndex_LocalBuffers;
    for (uint i = 0; i < (uint) myLocalBuffersToBind.size(); ++i)
    {
      if (myLocalBuffersToBind[i] == UINT64_MAX)
        continue;

      if (myCommandListType == CommandListType::Graphics || myCommandListType == CommandListType::Compute)
        myCommandList->SetComputeRootShaderResourceView(rootParamIdx + i, myLocalBuffersToBind[i]);
      if (myCommandListType == CommandListType::Graphics)
        myCommandList->SetGraphicsRootShaderResourceView(rootParamIdx + i, myLocalBuffersToBind[i]);
    }
    myLocalBuffersToBind.clear();

    rootParamIdx = rootSignature->myRootParamIndex_LocalRWBuffers;
    for (uint i = 0; i < (uint)myLocalRWBuffersToBind.size(); ++i)
    {
      if (myLocalRWBuffersToBind[i] == UINT64_MAX)
        continue;

      if (myCommandListType == CommandListType::Graphics || myCommandListType == CommandListType::Compute)
        myCommandList->SetComputeRootUnorderedAccessView(rootParamIdx + i, myLocalRWBuffersToBind[i]);
      if (myCommandListType == CommandListType::Graphics)
        myCommandList->SetGraphicsRootUnorderedAccessView(rootParamIdx + i, myLocalRWBuffersToBind[i]);
    }
    myLocalRWBuffersToBind.clear();

    rootParamIdx = rootSignature->myRootParamIndex_LocalCBuffers;
    for (uint i = 0; i < (uint)myLocalCBuffersToBind.size(); ++i)
    {
      if (myLocalCBuffersToBind[i] == UINT64_MAX)
        continue;

      if (myCommandListType == CommandListType::Graphics || myCommandListType == CommandListType::Compute)
        myCommandList->SetComputeRootConstantBufferView(rootParamIdx + i, myLocalCBuffersToBind[i]);
      if (myCommandListType == CommandListType::Graphics)
        myCommandList->SetGraphicsRootConstantBufferView(rootParamIdx + i, myLocalCBuffersToBind[i]);
    }
    myLocalCBuffersToBind.clear();
  }
//---------------------------------------------------------------------------//
  bool CommandListDX12::GetLocalSubresourceStates(const GpuResource* aResource, SubresourceLocation aSubresource, D3D12_RESOURCE_STATES& aStatesOut)
  {
    auto it = myLocalHazardData.find(aResource);
    if (it == myLocalHazardData.end())
      return false;

    const SubresourceHazardData& subData = it->second.mySubresources[aResource->GetSubresourceIndex(aSubresource)];
    if (!subData.myWasUsed)
      return false;

    aStatesOut = subData.myStates;

    return true;
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::TrackResourceTransition(const GpuResource* aResource, D3D12_RESOURCE_STATES aNewState, bool aIsSharedReadState /* = false */)
  {
    TrackSubresourceTransition(aResource, aResource->GetSubresources(), aNewState, aIsSharedReadState);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::TrackSubresourceTransition(const GpuResource* aResource, const SubresourceRange& aSubresourceRange, D3D12_RESOURCE_STATES aNewState, bool aToSharedReadState /* = false */)
  {
    if (aResource == nullptr)
      return;

    const bool canEarlyOut = !aToSharedReadState;

    GpuResourceHazardDataDX12& hazardData = aResource->GetDX12Data()->myHazardData;
    if (!hazardData.myCanChangeStates && canEarlyOut)
      return;

    D3D12_RESOURCE_STATES dstStates = ResolveValidateDstStates(aResource, aNewState);
    
    uint numPossibleSubresourceTransitions = 0u;
    eastl::fixed_vector<bool, 64> subresourceTransitionPossible(aSubresourceRange.GetNumSubresources(), false);

    uint i = 0u;
    for (SubresourceIterator it = aSubresourceRange.Begin(); it != aSubresourceRange.End(); ++it)
    {
      const bool transitionPossible = ValidateSubresourceTransition(aResource, aResource->GetSubresourceIndex(*it), dstStates);
      if (transitionPossible)
        ++numPossibleSubresourceTransitions;
      subresourceTransitionPossible[i++] = transitionPossible;
    }

    if (numPossibleSubresourceTransitions == 0u && canEarlyOut)
      return;

    const bool dstIsRead = (dstStates & DX12_READ_STATES) == dstStates;

    LocalHazardData* localData = nullptr;
    {
      auto it = myLocalHazardData.find(aResource);
      if (it == myLocalHazardData.end())  // We don't have a local record of this resource yet
      {
        localData = &myLocalHazardData[aResource];
        localData->mySubresources.resize(aResource->mySubresources.GetNumSubresources());
      }
      else
      {
        localData = &it->second;
      }
    }

    bool canTransitionAllSubresources = numPossibleSubresourceTransitions == aResource->mySubresources.GetNumSubresources();
    if (canTransitionAllSubresources)
    {
      const D3D12_RESOURCE_STATES firstSrcStates = localData->mySubresources[0].myStates;

      for (uint sub = 0u; canTransitionAllSubresources && sub < (uint) localData->mySubresources.size(); ++sub)
      {
        canTransitionAllSubresources &= localData->mySubresources[sub].myWasUsed &&
          localData->mySubresources[sub].myStates == firstSrcStates;
      }
    }

    if (canTransitionAllSubresources)
    {
      D3D12_RESOURCE_BARRIER barrier = {};
      barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
      barrier.Transition.StateBefore = localData->mySubresources[0].myStates;
      barrier.Transition.StateAfter = dstStates;
      barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
      barrier.Transition.pResource = aResource->GetDX12Data()->myResource.Get();
      AddBarrier(barrier);

#if FANCY_RENDERER_LOG_RESOURCE_BARRIERS
      if (RenderCore::ourDebugLogResourceBarriers)
        LOG_DEBUG("Subresource transition: %s (all subresources): %s -> %s", aResource->GetName(),
          DebugUtilsDX12::ResourceStatesToString(barrier.Transition.StateBefore).c_str(), DebugUtilsDX12::ResourceStatesToString(barrier.Transition.StateAfter).c_str());
#endif
    }
    else
    {
      i = 0u;
      for (SubresourceIterator it = aSubresourceRange.Begin(); it != aSubresourceRange.End(); ++it, ++i)
      {
        if (!subresourceTransitionPossible[i])
          continue;

        const uint subresourceIndex = aResource->GetSubresourceIndex(*it);

        SubresourceHazardData& subData = localData->mySubresources[subresourceIndex];
        if (subData.myWasUsed)
        {
          D3D12_RESOURCE_BARRIER barrier = {};
          barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
          barrier.Transition.StateBefore = subData.myStates;
          barrier.Transition.StateAfter = dstStates;
          barrier.Transition.Subresource = subresourceIndex;
          barrier.Transition.pResource = aResource->GetDX12Data()->myResource.Get();
          AddBarrier(barrier);

#if FANCY_RENDERER_LOG_RESOURCE_BARRIERS
          if (RenderCore::ourDebugLogResourceBarriers)
            LOG_DEBUG("Subresource transition: %s (subresource %d): %s -> %s", aResource->GetName(), subresourceIndex,
              DebugUtilsDX12::ResourceStatesToString(barrier.Transition.StateBefore).c_str(), DebugUtilsDX12::ResourceStatesToString(barrier.Transition.StateAfter).c_str());
#endif
        }
      }
    }

    i = 0u;
    for (SubresourceIterator it = aSubresourceRange.Begin(); it != aSubresourceRange.End(); ++it, ++i)
    {
      const uint subresourceIndex = aResource->GetSubresourceIndex(*it);
      SubresourceHazardData& subData = localData->mySubresources[subresourceIndex];
      if (aToSharedReadState)
      {
        subData.myWasWritten = false;
        subData.myIsSharedReadState = true;
      }

      if (!subresourceTransitionPossible[i])
        continue;

      if (!subData.myWasUsed)
      {
#if FANCY_RENDERER_LOG_RESOURCE_BARRIERS
        if (RenderCore::ourDebugLogResourceBarriers)
          LOG_DEBUG("Open subresource transition: %s (subresource %d): ? -> %s", aResource->GetName(), subresourceIndex, DebugUtilsDX12::ResourceStatesToString(dstStates).c_str());
#endif
        subData.myFirstDstStates = dstStates;
      }

      subData.myWasUsed = true;
      subData.myStates = dstStates;

      if (!dstIsRead)
      {
        subData.myWasWritten = true;
        subData.myIsSharedReadState = false;
      }
    }
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::AddBarrier(const D3D12_RESOURCE_BARRIER& aBarrier)
  {
    if (myPendingBarriers.full())
      FlushBarriers();
    myPendingBarriers.push_back(aBarrier);
  }
//---------------------------------------------------------------------------//
  D3D12_RESOURCE_STATES CommandListDX12::ResolveValidateDstStates(const GpuResource* aResource, D3D12_RESOURCE_STATES aDstStates)
  {
    bool wasEmpty = aDstStates == (D3D12_RESOURCE_STATES) 0;

    D3D12_RESOURCE_STATES dstStates = aDstStates & myResourceStateMask;
    ASSERT(wasEmpty || dstStates != 0, "Unsupported resource states for this commandlist type");
    ASSERT((dstStates & DX12_READ_STATES) == dstStates || (dstStates & DX12_WRITE_STATES) == dstStates, "Simulataneous read- and write states are not allowed");

    const bool dstIsRead = (dstStates & DX12_READ_STATES) == dstStates;
    dstStates = (dstIsRead ? aResource->GetDX12Data()->myHazardData.myReadStates : aResource->GetDX12Data()->myHazardData.myWriteStates) & dstStates;
    ASSERT(wasEmpty || dstStates != 0u, "Dst resource states not supported by resource");

    return dstStates;
  }
//---------------------------------------------------------------------------//
  bool CommandListDX12::ValidateSubresourceTransition(const GpuResource* aResource, uint aSubresourceIndex, D3D12_RESOURCE_STATES aDstStates)
  {
    const GpuResourceHazardDataDX12& globalData = aResource->GetDX12Data()->myHazardData;

    D3D12_RESOURCE_STATES currStates = (D3D12_RESOURCE_STATES) globalData.mySubresources[aSubresourceIndex].myStates;
    CommandListType currGlobalContext = globalData.mySubresources[aSubresourceIndex].myContext;

    auto it = myLocalHazardData.find(aResource);
    const bool hasLocalData = it != myLocalHazardData.end();
    if (hasLocalData)
      currStates = it->second.mySubresources[aSubresourceIndex].myStates;

    bool currStateHasAllDstStates = (currStates & aDstStates) == aDstStates;
    if (aDstStates == D3D12_RESOURCE_STATE_COMMON)
      currStateHasAllDstStates = currStates == D3D12_RESOURCE_STATE_COMMON;

    const bool dstIsRead = (aDstStates & DX12_READ_STATES) == aDstStates;
    bool isInSharedReadState = dstIsRead && currGlobalContext == CommandListType::SHARED_READ;
    if (hasLocalData && it->second.mySubresources[aSubresourceIndex].myWasWritten)
    {
      // The subresource left the shared read state in this command list
      isInSharedReadState = false;
    }

    // We can only truly skip this transition if we already have the resource state on the local timeline. 
    // If the subresource is on the shared read context and the destination is a read state, we can also skip since the subresource is not expected to transition at all
    if (currStateHasAllDstStates && (hasLocalData || isInSharedReadState)) 
      return false;

    // If we reached this point it means that a state is missing in the curr state and we need to add a barrier for this subresource. However, it is an error to transition 
    // To another read-state if the resource is currently being used by multiple queues. A write-state would take ownership of this resource again however
    if (isInSharedReadState)
    {
      ASSERT(false, "No resource transitions allowed on SHARED_READ context. Resource must be transitioned to a state mask that incorporates all future read-states");
      return false;
    }

    return true;
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::ApplyGraphicsPipelineState()
  {
    if (!myGraphicsPipelineState.myIsDirty)
      return;

    myGraphicsPipelineState.myIsDirty = false;

    ID3D12PipelineState* pso = RenderCore::GetPlatformDX12()->GetPipelineStateCache().GetCreateGraphicsPSO(myGraphicsPipelineState);
    myCommandList->SetPipelineState(pso);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::ApplyComputePipelineState()
  {
    if (!myComputePipelineState.myIsDirty)
      return;

    myComputePipelineState.myIsDirty = false;
    
    ID3D12PipelineState* pso = RenderCore::GetPlatformDX12()->GetPipelineStateCache().GetCreateComputePSO(myComputePipelineState);
    myCommandList->SetPipelineState(pso);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::ApplyRaytracingPipelineState()
  {
    if (!myRaytracingPipelineStateDirty)
      return;

    myRaytracingPipelineStateDirty = false;

    RtPipelineStateDX12* rtPsoDx12 = static_cast<RtPipelineStateDX12*>(myRaytracingPipelineState);
    myCommandList->SetPipelineState1(rtPsoDx12->myStateObject.Get());
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::Dispatch(const glm::int3& aNumThreads)
  {
    ApplyComputePipelineState();
    ApplyResourceBindings();
    FlushBarriers();
    ASSERT(myComputePipelineState.myShaderPipeline != nullptr);

    const Shader* shader = myComputePipelineState.myShaderPipeline->GetShader(ShaderStage::SHADERSTAGE_COMPUTE);
    ASSERT(shader != nullptr);

    const glm::int3& numGroupThreads = shader->GetProperties().myNumGroupThreads;
    const glm::int3 numGroups = glm::max(glm::int3(1), aNumThreads / numGroupThreads);
    myCommandList->Dispatch(static_cast<uint>(numGroups.x), static_cast<uint>(numGroups.y), static_cast<uint>(numGroups.z));
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::DispatchRays(const DispatchRaysDesc& aDesc)
  {
    ASSERT(aDesc.myRayGenShaderTableRange.mySbtBuffer != nullptr);

    ApplyRaytracingPipelineState();
    ASSERT(myRaytracingPipelineState != nullptr);

    TrackResourceTransition(aDesc.myRayGenShaderTableRange.mySbtBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    TrackResourceTransition(aDesc.myCallableShaderTableRange.mySbtBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    TrackResourceTransition(aDesc.myMissShaderTableRange.mySbtBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    TrackResourceTransition(aDesc.myHitGroupTableRange.mySbtBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    ApplyResourceBindings();
    FlushBarriers();
    
    D3D12_DISPATCH_RAYS_DESC desc = {};
    desc.Width = aDesc.myWidth;
    desc.Height = aDesc.myHeight;
    desc.Depth = aDesc.myDepth;

    desc.RayGenerationShaderRecord.StartAddress = aDesc.myRayGenShaderTableRange.mySbtBuffer->GetDeviceAddress() + aDesc.myRayGenShaderTableRange.myOffset;
    desc.RayGenerationShaderRecord.SizeInBytes = aDesc.myRayGenShaderTableRange.mySize;

    if (aDesc.myMissShaderTableRange.mySbtBuffer != nullptr)
    {
      desc.MissShaderTable.StartAddress = aDesc.myMissShaderTableRange.mySbtBuffer->GetDeviceAddress() + aDesc.myMissShaderTableRange.myOffset;
      desc.MissShaderTable.SizeInBytes = aDesc.myMissShaderTableRange.mySize;
      desc.MissShaderTable.StrideInBytes = aDesc.myMissShaderTableRange.myStride;
    }

    if (aDesc.myHitGroupTableRange.mySbtBuffer != nullptr)
    {
      desc.HitGroupTable.StartAddress = aDesc.myHitGroupTableRange.mySbtBuffer->GetDeviceAddress() + aDesc.myHitGroupTableRange.myOffset;
      desc.HitGroupTable.SizeInBytes = aDesc.myHitGroupTableRange.mySize;
      desc.HitGroupTable.StrideInBytes = aDesc.myHitGroupTableRange.myStride;
    }

    if (aDesc.myCallableShaderTableRange.mySbtBuffer != nullptr)
    {
      desc.CallableShaderTable.StartAddress = aDesc.myCallableShaderTableRange.mySbtBuffer->GetDeviceAddress() + aDesc.myCallableShaderTableRange.myOffset;
      desc.CallableShaderTable.SizeInBytes = aDesc.myCallableShaderTableRange.mySize;
      desc.CallableShaderTable.StrideInBytes = aDesc.myCallableShaderTableRange.myStride;
    }

    myCommandList->DispatchRays(&desc);
  }
//---------------------------------------------------------------------------//
  void CommandListDX12::Close()
  {
    if (myIsOpen)
      ASSERT_HRESULT(myCommandList->Close());

    myIsOpen = false;
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
}

#endif