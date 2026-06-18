#include "fancy_core_precompile.h"
#include "CommandListDX12.h"

#if FANCY_ENABLE_DX12

#include "Common/FancyCoreDefines.h"
#include "Common/TimeManager.h"

#include "Rendering/RenderCore.h"
#include "Rendering/BlendState.h"

#include "GpuBufferDX12.h"
#include "RenderCore_PlatformDX12.h"
#include "DX12Prerequisites.h"
#include "ResourceBarrierStatesDX12.h"
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
    D3D12_COMMAND_LIST_TYPE locResolveCommandListType( CommandListType aCommandListType ) {
      switch ( aCommandListType ) {
        case CommandListType::Graphics:
          return D3D12_COMMAND_LIST_TYPE_DIRECT;
        case CommandListType::Compute:
          return D3D12_COMMAND_LIST_TYPE_COMPUTE;
        case CommandListType::DMA:
          return D3D12_COMMAND_LIST_TYPE_COPY;
        default:
          ASSERT( false, "CommandListType %d not implemented", ( uint ) aCommandListType );
          return D3D12_COMMAND_LIST_TYPE_DIRECT;
      }
    }
    //---------------------------------------------------------------------------//
    const uint locBarrierAllSubresources = 0xffffffffu;
    //---------------------------------------------------------------------------//
    D3D12_BARRIER_SUBRESOURCE_RANGE locResolveAllBarrierSubresourceRange() {
      D3D12_BARRIER_SUBRESOURCE_RANGE range = {};
      range.IndexOrFirstMipLevel = 0u;
      range.NumMipLevels = locBarrierAllSubresources;
      range.FirstArraySlice = 0u;
      range.NumArraySlices = locBarrierAllSubresources;
      range.FirstPlane = 0u;
      range.NumPlanes = locBarrierAllSubresources;
      return range;
    }
    //---------------------------------------------------------------------------//
    D3D12_BARRIER_SUBRESOURCE_RANGE locResolveBarrierSubresourceRange( const SubresourceRange & aRange ) {
      D3D12_BARRIER_SUBRESOURCE_RANGE range = {};
      range.IndexOrFirstMipLevel = aRange.myFirstMipLevel;
      range.NumMipLevels = aRange.myNumMipLevels;
      range.FirstArraySlice = aRange.myFirstArrayIndex;
      range.NumArraySlices = aRange.myNumArrayIndices;
      range.FirstPlane = aRange.myFirstPlane;
      range.NumPlanes = aRange.myNumPlanes;
      return range;
    }
    //---------------------------------------------------------------------------//
    D3D12_BARRIER_SUBRESOURCE_RANGE locResolveBarrierSubresourceRange( const SubresourceLocation & aLocation ) {
      D3D12_BARRIER_SUBRESOURCE_RANGE range = {};
      range.IndexOrFirstMipLevel = aLocation.myMipLevel;
      range.NumMipLevels = 1u;
      range.FirstArraySlice = aLocation.myArrayIndex;
      range.NumArraySlices = 1u;
      range.FirstPlane = aLocation.myPlaneIndex;
      range.NumPlanes = 1u;
      return range;
    }
  }  // namespace
  //---------------------------------------------------------------------------//

  //---------------------------------------------------------------------------//
  CommandListDX12::CommandListDX12( CommandListType aCommandListType )
      : CommandList( aCommandListType ), myCommandList( nullptr ), myCommandAllocator( nullptr ) {
    myCommandAllocator = RenderCore::GetPlatformDX12()->GetCommandAllocator( myCommandListType );

    D3D12_COMMAND_LIST_TYPE nativeCmdListType = locResolveCommandListType( aCommandListType );
    ID3D12GraphicsCommandList7 * cmdList7 = nullptr;

    ASSERT_HRESULT( RenderCore::GetPlatformDX12()->GetDevice()->CreateCommandList(
        0, nativeCmdListType, myCommandAllocator, nullptr, IID_PPV_ARGS( &cmdList7 ) ) );
    myCommandList = cmdList7;

    PrepareForRecord( false );
  }
  //---------------------------------------------------------------------------//
  CommandListDX12::~CommandListDX12() {
    CommandListDX12::PostExecute( 0ull );

    if ( myCommandList != nullptr )
      myCommandList->Release();

    myCommandList = nullptr;

    if ( myCommandAllocator != nullptr )
      RenderCore::GetPlatformDX12()->ReleaseCommandAllocator( myCommandAllocator, 0ull );
  }
  //---------------------------------------------------------------------------//
  D3D12_DESCRIPTOR_HEAP_TYPE CommandListDX12::ResolveDescriptorHeapTypeFromMask( uint aDescriptorTypeMask ) {
    if ( aDescriptorTypeMask & ( uint ) GpuDescriptorTypeFlags::BUFFER_TEXTURE_CONSTANT_BUFFER ) {
      return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    }
    else if ( aDescriptorTypeMask & ( uint ) GpuDescriptorTypeFlags::SAMPLER ) {
      return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    }

    ASSERT( false, "unsupported descriptor type mask" );
    return ( D3D12_DESCRIPTOR_HEAP_TYPE ) -1;
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::PrepareForRecord( bool aResetCommandList ) {
    RenderCore_PlatformDX12 * platformDx12 = RenderCore::GetPlatformDX12();

    if ( aResetCommandList )
      ASSERT_HRESULT( myCommandList->Reset( myCommandAllocator, nullptr ) );

    myTopologyDirty = true;
    myPendingTextureBarriers.clear();
    myPendingBufferBarriers.clear();
    myLocalHazardData.clear();

    const ShaderVisibleDescriptorHeapDX12 * shaderVisibleHeap = platformDx12->GetShaderVisibleDescriptorHeap();

    // We only use one shader-visible descriptor heap per type, so just bind them up-front
    ID3D12DescriptorHeap * shaderVisibleHeaps[] = { shaderVisibleHeap->GetResourceHeap(),
                                                    shaderVisibleHeap->GetSamplerHeap() };
    myCommandList->SetDescriptorHeaps( ARRAY_LENGTH( shaderVisibleHeaps ), shaderVisibleHeaps );

    // Set the root signature up front since we only use one
    const RootSignatureDX12 * rootSignature = platformDx12->GetRootSignature();
    if ( myCommandListType == CommandListType::Graphics || myCommandListType == CommandListType::Compute ) {
      myCommandList->SetComputeRootSignature( rootSignature->GetRootSignature() );
      myCommandList->SetComputeRootDescriptorTable( rootSignature->myRootParamIndex_GlobalResources,
                                                    shaderVisibleHeap->GetResourceHeapStart() );
      myCommandList->SetComputeRootDescriptorTable( rootSignature->myRootParamIndex_GlobalSamplers,
                                                    shaderVisibleHeap->GetSamplerHeapStart() );
    }

    if ( myCommandListType == CommandListType::Graphics ) {
      myCommandList->SetGraphicsRootSignature( rootSignature->GetRootSignature() );
      myCommandList->SetGraphicsRootDescriptorTable( rootSignature->myRootParamIndex_GlobalResources,
                                                     shaderVisibleHeap->GetResourceHeapStart() );
      myCommandList->SetGraphicsRootDescriptorTable( rootSignature->myRootParamIndex_GlobalSamplers,
                                                     shaderVisibleHeap->GetSamplerHeapStart() );
    }
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::UpdateSubresources( ID3D12Resource * aDstResource, ID3D12Resource * aStagingResource,
                                            uint aFirstSubresourceIndex, uint aNumSubresources,
                                            D3D12_SUBRESOURCE_DATA * someSubresourceDatas ) {
    D3D12_RESOURCE_DESC srcDesc = aStagingResource->GetDesc();
    D3D12_RESOURCE_DESC destDesc = aDstResource->GetDesc();

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT * destLayouts = static_cast< D3D12_PLACED_SUBRESOURCE_FOOTPRINT * >(
        alloca( sizeof( D3D12_PLACED_SUBRESOURCE_FOOTPRINT ) * aNumSubresources ) );
    uint64 * destRowSizesByte = static_cast< uint64 * >( alloca( sizeof( uint64 ) * aNumSubresources ) );
    uint *   destRowNums = static_cast< uint * >( alloca( sizeof( uint ) * aNumSubresources ) );

    uint64 destTotalSizeBytes = 0u;
    RenderCore::GetPlatformDX12()->GetDevice()->GetCopyableFootprints( &destDesc, aFirstSubresourceIndex,
                                                                       aNumSubresources, 0u, destLayouts, destRowNums,
                                                                       destRowSizesByte, &destTotalSizeBytes );

    FlushBarriers();

    // Prepare a temporary buffer that contains all subresource data in the expected form (i.e. respecting the dest data
    // layout)
    uint8 * tempBufferDataPtr;
    if ( S_OK != aStagingResource->Map( 0, nullptr, reinterpret_cast< void ** >( &tempBufferDataPtr ) ) ) {
      return;
    }

    for ( uint i = 0u; i < aNumSubresources; ++i ) {
      uint8 * dstSubResourceData = tempBufferDataPtr + destLayouts[ i ].Offset;
      uint64  dstSubResourceRowSize = destLayouts[ i ].Footprint.RowPitch;
      uint64  dstSubResourceSliceSize = dstSubResourceRowSize * destRowNums[ i ];

      uint8 * srcSubResourceData = ( uint8 * ) someSubresourceDatas[ i ].pData;
      uint64  srcSubResourceRowSize = someSubresourceDatas[ i ].RowPitch;
      uint64  srcSubResourceSliceSize = someSubresourceDatas[ i ].SlicePitch;

      for ( uint iSlice = 0u; iSlice < destLayouts[ i ].Footprint.Depth; ++iSlice ) {
        uint8 *       destSliceDataPtr = dstSubResourceData + dstSubResourceSliceSize * iSlice;
        const uint8 * srcSliceDataPtr = srcSubResourceData + srcSubResourceSliceSize * iSlice;
        for ( uint iRow = 0u; iRow < destRowNums[ i ]; ++iRow ) {
          uint8 *       destDataPtr = destSliceDataPtr + dstSubResourceRowSize * iRow;
          const uint8 * srcDataPtr = srcSliceDataPtr + srcSubResourceRowSize * iRow;

          memcpy( destDataPtr, srcDataPtr, destRowSizesByte[ i ] );
        }
      }
    }
    aStagingResource->Unmap( 0, nullptr );

    // Copy from the temp staging buffer to the destination resource (could be buffer or texture)
    if ( destDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER ) {
      myCommandList->CopyBufferRegion( aDstResource, 0, aStagingResource, destLayouts[ 0 ].Offset,
                                       destLayouts[ 0 ].Footprint.Width );
    } else {
      for ( uint i = 0u; i < aNumSubresources; ++i ) {
        D3D12_TEXTURE_COPY_LOCATION destCopyLocation;
        destCopyLocation.pResource = aDstResource;
        destCopyLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        destCopyLocation.SubresourceIndex = aFirstSubresourceIndex + i;

        D3D12_TEXTURE_COPY_LOCATION srcCopyLocation;
        srcCopyLocation.pResource = aStagingResource;
        srcCopyLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        srcCopyLocation.PlacedFootprint = destLayouts[ i ];
        myCommandList->CopyTextureRegion( &destCopyLocation, 0u, 0u, 0u, &srcCopyLocation, nullptr );
      }
    }
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::ClearRenderTarget( TextureView * aTextureView, const float * aColor ) {
    const GpuResourceViewDataDX12 & viewDataDx12 = aTextureView->myDX12Data;

    ASSERT( aTextureView->GetProperties().myIsRenderTarget );
    ASSERT( aTextureView->myType == GpuResourceViewType::RTV );

    TrackTextureTransition( aTextureView->GetTexture(), GPU_TEXTURE_STATE_RENDER_TARGET );
    FlushBarriers();

    myCommandList->ClearRenderTargetView( viewDataDx12.myDescriptor.myCpuHandle, aColor, 0, nullptr );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::ClearDepthStencilTarget( TextureView * aTextureView, float aDepthClear, uint8 aStencilClear,
                                                 uint someClearFlags ) {
    const GpuResourceViewDataDX12 & viewDataDx12 = aTextureView->myDX12Data;
    ASSERT( aTextureView->myType == GpuResourceViewType::DSV );

    const DataFormat       format = aTextureView->GetTexture()->GetProperties().myFormat;
    const DataFormatInfo & formatInfo = DataFormatInfo::GetFormatInfo( format );
    ASSERT( formatInfo.myIsDepthStencil );

    const bool clearDepth = someClearFlags & ( uint ) DepthStencilClearFlags::CLEAR_DEPTH;
    const bool clearStencil = someClearFlags & ( uint ) DepthStencilClearFlags::CLEAR_STENCIL;

    const SubresourceRange & subresources = aTextureView->GetSubresourceRange();

    if ( clearDepth && !clearStencil ) {
      ASSERT( subresources.myFirstPlane == 0, "The texture view doesn't cover the depth plane" );
    } else if ( clearStencil && !clearDepth ) {
      ASSERT( formatInfo.myNumPlanes == 2 );
      ASSERT( subresources.myFirstPlane + subresources.myNumPlanes >= 2,
              "The texture view doesn't cover the stencil plane" );
    } else {
      ASSERT( formatInfo.myNumPlanes == 2 );
      ASSERT( subresources.myFirstPlane == 0, "The texture view doesn't cover the depth plane" );
      ASSERT( subresources.myFirstPlane + subresources.myNumPlanes >= 2,
              "The texture view doesn't cover the stencil plane" );
    }

    TrackTextureTransition( aTextureView->GetTexture(), GPU_TEXTURE_STATE_DEPTH_WRITE );
    FlushBarriers();

    D3D12_CLEAR_FLAGS clearFlags = ( D3D12_CLEAR_FLAGS ) 0;
    if ( someClearFlags & ( uint ) DepthStencilClearFlags::CLEAR_DEPTH )
      clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
    if ( someClearFlags & ( uint ) DepthStencilClearFlags::CLEAR_STENCIL )
      clearFlags |= D3D12_CLEAR_FLAG_STENCIL;

    myCommandList->ClearDepthStencilView( viewDataDx12.myDescriptor.myCpuHandle, clearFlags, aDepthClear, aStencilClear,
                                          0, nullptr );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::CopyResource( GpuResource * aDstResource, GpuResource * aSrcResource ) {
    if ( aSrcResource->IsTexture() ) {
      TrackTextureTransition( aSrcResource, GPU_TEXTURE_STATE_COPY_SOURCE );
    }
    else {
      TrackBufferTransition( aSrcResource, GPU_BUFFER_STATE_COPY_SOURCE );
    }

    if ( aDstResource->IsTexture() ) {
      TrackTextureTransition( aDstResource, GPU_TEXTURE_STATE_COPY_DEST );
    }
    else {
      TrackBufferTransition( aDstResource, GPU_BUFFER_STATE_COPY_DEST );
    }

    FlushBarriers();

    GpuResourceDataDX12 * destData = aDstResource->GetDX12Data();
    GpuResourceDataDX12 * srcData = aSrcResource->GetDX12Data();

    myCommandList->CopyResource( destData->myResource.Get(), srcData->myResource.Get() );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::CopyBuffer( const GpuBuffer * aDstBuffer, uint64 aDstOffset, const GpuBuffer * aSrcBuffer,
                                    uint64 aSrcOffset, uint64 aSize ) {
    ASSERT( aDstBuffer != aSrcBuffer, "Copying within the same buffer is not supported (same subresource)" );

#if FANCY_RENDERER_USE_VALIDATION
    ValidateBufferCopy( aDstBuffer->GetProperties(), aDstOffset, aSrcBuffer->GetProperties(), aSrcOffset, aSize );
#endif

    TrackBufferTransition( aSrcBuffer, GPU_BUFFER_STATE_COPY_SOURCE );
    TrackBufferTransition( aDstBuffer, GPU_BUFFER_STATE_COPY_DEST );

    FlushBarriers();

    ID3D12Resource * dstResource = aDstBuffer->GetDX12Data()->myResource.Get();
    ID3D12Resource * srcResource = aSrcBuffer->GetDX12Data()->myResource.Get();

    myCommandList->CopyBufferRegion( dstResource, aDstOffset, srcResource, aSrcOffset, aSize );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::CopyTextureToBuffer( const GpuBuffer * aDstBuffer, uint64 aDstOffset,
                                             const Texture * aSrcTexture, const SubresourceLocation & aSrcSubresource,
                                             const TextureRegion & aSrcRegion ) {
#if FANCY_RENDERER_USE_VALIDATION
    ValidateTextureToBufferCopy( aDstBuffer->GetProperties(), aDstOffset, aSrcTexture->GetProperties(), aSrcSubresource,
                                 aSrcRegion );
#endif

    ID3D12Resource * bufferResourceDX12 = aDstBuffer->GetDX12Data()->myResource.Get();
    ID3D12Resource * textureResourceDX12 = aSrcTexture->GetDX12Data()->myResource.Get();

    const uint16 textureSubresourceIndex = static_cast< uint16 >( aSrcTexture->GetSubresourceIndex( aSrcSubresource ) );

    TrackTextureSubresourceTransition( aSrcTexture, SubresourceRange( aSrcSubresource ),
                                       GPU_TEXTURE_STATE_COPY_SOURCE );
    TrackBufferTransition( aDstBuffer, GPU_BUFFER_STATE_COPY_DEST );

    D3D12_TEXTURE_COPY_LOCATION srcLocation;
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    srcLocation.SubresourceIndex = textureSubresourceIndex;
    srcLocation.pResource = textureResourceDX12;

    const DataFormat       format = aSrcTexture->GetProperties().myFormat;
    const DataFormatInfo & formatInfo = DataFormatInfo::GetFormatInfo( format );
    const DXGI_FORMAT      formatDx12 = RenderCore_PlatformDX12::ResolveFormat( format );

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
    footprint.Offset = 0;
    footprint.Footprint.Format = RenderCore_PlatformDX12::GetCopyableFormat( formatDx12, aSrcSubresource.myPlaneIndex );
    footprint.Footprint.Width = aSrcRegion.mySize.x;
    footprint.Footprint.RowPitch = ( uint ) MathUtil::Align(
        ( uint64 ) BITS_TO_BYTES( aSrcRegion.mySize.x *
                                  formatInfo.myCopyableBitsPerPixelPerPlane[ aSrcSubresource.myPlaneIndex ] ),
        ( uint64 ) RenderCore::GetPlatformCaps().myTextureRowAlignment );
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
    myCommandList->CopyTextureRegion( &destLocation, 0, 0, 0, &srcLocation, &srcBox );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::CopyTexture( const Texture * aDstTexture, const SubresourceLocation & aDstSubresource,
                                     const TextureRegion & aDstRegion, const Texture * aSrcTexture,
                                     const SubresourceLocation & aSrcSubresource, const TextureRegion & aSrcRegion ) {
#if FANCY_RENDERER_USE_VALIDATION
    ValidateTextureCopy( aDstTexture->GetProperties(), aDstSubresource, aDstRegion, aSrcTexture->GetProperties(),
                         aSrcSubresource, aSrcRegion );
#endif

    ID3D12Resource * dstResource = aDstTexture->GetDX12Data()->myResource.Get();
    ID3D12Resource * srcResource = aSrcTexture->GetDX12Data()->myResource.Get();

    const uint16 destSubResourceIndex = static_cast< uint16 >( aDstTexture->GetSubresourceIndex( aDstSubresource ) );
    const uint16 srcSubResourceIndex = static_cast< uint16 >( aSrcTexture->GetSubresourceIndex( aSrcSubresource ) );

    TrackTextureSubresourceTransition( aSrcTexture, SubresourceRange( aSrcSubresource ),
                                       GPU_TEXTURE_STATE_COPY_SOURCE );
    TrackTextureSubresourceTransition( aDstTexture, SubresourceRange( aDstSubresource ),
                                       GPU_TEXTURE_STATE_COPY_DEST );

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
    myCommandList->CopyTextureRegion( &dstLocation, aDstRegion.myPos.x, aDstRegion.myPos.y, aDstRegion.myPos.z,
                                      &srcLocation, &srcBox );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::CopyBufferToTexture( const Texture * aDstTexture, const SubresourceLocation & aDstSubresource,
                                             const TextureRegion & aDstRegion, const GpuBuffer * aSrcBuffer,
                                             uint64 aSrcOffset ) {
#if FANCY_RENDERER_USE_VALIDATION
    ValidateBufferToTextureCopy( aDstTexture->GetProperties(), aDstSubresource, aDstRegion, aSrcBuffer->GetProperties(),
                                 aSrcOffset );
#endif

    ID3D12Resource * dstResource = aDstTexture->GetDX12Data()->myResource.Get();
    ID3D12Resource * srcResource = aSrcBuffer->GetDX12Data()->myResource.Get();

    const uint16 destSubResourceIndex = static_cast< uint16 >( aDstTexture->GetSubresourceIndex( aDstSubresource ) );
    const uint16 srcSubResourceIndex = 0;

    TrackBufferTransition( aSrcBuffer, GPU_BUFFER_STATE_COPY_SOURCE );
    TrackTextureSubresourceTransition( aDstTexture, SubresourceRange( aDstSubresource ),
                                       GPU_TEXTURE_STATE_COPY_DEST );

    D3D12_TEXTURE_COPY_LOCATION dstLocation;
    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLocation.SubresourceIndex = destSubResourceIndex;
    dstLocation.pResource = dstResource;

    const DataFormat       format = aDstTexture->GetProperties().myFormat;
    const DataFormatInfo & formatInfo = DataFormatInfo::GetFormatInfo( format );
    const DXGI_FORMAT      formatDx12 = RenderCore_PlatformDX12::ResolveFormat( format );

    uint64 rowPitch;
    uint   heightBlocksOrPixel;
    TextureData::ComputeRowPitchSizeAndBlockHeight( format, aDstRegion.mySize.x, aDstRegion.mySize.y, rowPitch,
                                                    heightBlocksOrPixel, aDstSubresource.myPlaneIndex );

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
    footprint.Offset = 0u;
    footprint.Footprint.Format = RenderCore_PlatformDX12::GetCopyableFormat( formatDx12, aDstSubresource.myPlaneIndex );
    footprint.Footprint.Width = aDstRegion.mySize.x;
    footprint.Footprint.RowPitch =
        ( uint ) MathUtil::Align( rowPitch, RenderCore::GetPlatformCaps().myTextureRowAlignment );
    footprint.Footprint.Height = aDstRegion.mySize.y;
    footprint.Footprint.Depth = aDstRegion.mySize.z;

    D3D12_TEXTURE_COPY_LOCATION srcLocation;
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    srcLocation.pResource = srcResource;
    srcLocation.PlacedFootprint.Footprint = footprint.Footprint;
    srcLocation.PlacedFootprint.Offset = aSrcOffset;

    FlushBarriers();

    myCommandList->CopyTextureRegion( &dstLocation, aDstRegion.myPos.x, aDstRegion.myPos.y, aDstRegion.myPos.z,
                                      &srcLocation, nullptr );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::UpdateTextureData( const Texture * aDstTexture, const SubresourceRange & aSubresourceRange,
                                           const TextureSubData * someDatas, uint aNumDatas ) {
    const uint numSubresources = aSubresourceRange.GetNumSubresources();
    ASSERT( aNumDatas == numSubresources );

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT * footprints = ( D3D12_PLACED_SUBRESOURCE_FOOTPRINT * ) alloca(
        sizeof( D3D12_PLACED_SUBRESOURCE_FOOTPRINT ) * numSubresources );
    uint *   rowNums = ( uint * ) alloca( sizeof( uint ) * numSubresources );
    uint64 * rowSizes = ( uint64 * ) alloca( sizeof( uint64 ) * numSubresources );
    uint64   totalSize = static_cast< const TextureDX12 * >( aDstTexture )
                             ->GetCopyableFootprints( aSubresourceRange, footprints, rowNums, rowSizes );

    uint64            uploadBufferOffset;
    const GpuBuffer * uploadBuffer =
        GetBuffer( uploadBufferOffset, GpuBufferUsage::STAGING_UPLOAD, nullptr, totalSize );
    ASSERT( uploadBuffer != nullptr );

    uint8 * uploadBufferData =
        ( uint8 * ) uploadBuffer->Map( GpuResourceMapMode::WRITE_UNSYNCHRONIZED, uploadBufferOffset, totalSize );

    for ( uint i = 0; i < aNumDatas; ++i ) {
      const D3D12_PLACED_SUBRESOURCE_FOOTPRINT & footprint = footprints[ i ];
      uint                                       numRows = rowNums[ i ];

      const TextureSubData & srcData = someDatas[ i ];
      ASSERT( rowSizes[ i ] == srcData.myRowSizeBytes );

      const uint64 alignedSliceSize = footprint.Footprint.RowPitch * numRows;

      uint8 *       dstSubresourceData = uploadBufferData + footprint.Offset;
      const uint8 * srcSubresourceData = srcData.myData;
      for ( uint iSlice = 0; iSlice < footprint.Footprint.Depth; ++iSlice ) {
        uint8 *       dstSliceData = dstSubresourceData + iSlice * alignedSliceSize;
        const uint8 * srcSliceData = srcSubresourceData + iSlice * srcData.mySliceSizeBytes;

        for ( uint iRow = 0; iRow < numRows; ++iRow ) {
          uint8 *       dstRowData = dstSliceData + iRow * footprint.Footprint.RowPitch;
          const uint8 * srcRowData = srcSliceData + iRow * srcData.myRowSizeBytes;

          memcpy( dstRowData, srcRowData, srcData.myRowSizeBytes );
        }
      }
    }
    uploadBuffer->Unmap( GpuResourceMapMode::WRITE_UNSYNCHRONIZED, uploadBufferOffset, totalSize );

    TrackTextureTransition( aDstTexture, GPU_TEXTURE_STATE_COPY_DEST );

    int i = 0;
    for ( SubresourceIterator subIter = aSubresourceRange.Begin(), e = aSubresourceRange.End(); subIter != e;
          ++subIter ) {
      const SubresourceLocation dstLocation = *subIter;
      CommandList::CopyBufferToTexture( aDstTexture, dstLocation, uploadBuffer,
                                        uploadBufferOffset + footprints[ i++ ].Offset );
    }
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::PostExecute( uint64 aFenceVal ) {
    CommandList::PostExecute( aFenceVal );

    if ( myCommandAllocator != nullptr )
      RenderCore::GetPlatformDX12()->ReleaseCommandAllocator( myCommandAllocator, aFenceVal );
    myCommandAllocator = nullptr;

    myLocalHazardData.clear();
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::ResetAndOpen() {
    CommandList::ResetAndOpen();

    RenderCore_PlatformDX12 * platformDx12 = RenderCore::GetPlatformDX12();
    myCommandAllocator = platformDx12->GetCommandAllocator( myCommandListType );
    ASSERT( myCommandAllocator != nullptr );

    PrepareForRecord( true );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::FlushBarriers() {
    if ( myPendingTextureBarriers.empty() && myPendingBufferBarriers.empty() )
      return;

    ASSERT( RenderCore::GetPlatformDX12()->SupportsEnhancedBarriers(), "Device does not support enhanced barriers. Enhanced barriers must be enabled." );

    D3D12_BARRIER_GROUP barrierGroups[ 2 ];
    uint                numGroups = 0u;

    if ( !myPendingTextureBarriers.empty() ) {
      D3D12_BARRIER_GROUP & group = barrierGroups[ numGroups++ ];
      group.Type = D3D12_BARRIER_TYPE_TEXTURE;
      group.NumBarriers = ( uint ) myPendingTextureBarriers.size();
      group.pTextureBarriers = myPendingTextureBarriers.data();
    }
    if ( !myPendingBufferBarriers.empty() ) {
      D3D12_BARRIER_GROUP & group = barrierGroups[ numGroups++ ];
      group.Type = D3D12_BARRIER_TYPE_BUFFER;
      group.NumBarriers = ( uint ) myPendingBufferBarriers.size();
      group.pBufferBarriers = myPendingBufferBarriers.data();
    }

    myCommandList->Barrier( numGroups, barrierGroups );
    myPendingTextureBarriers.clear();
    myPendingBufferBarriers.clear();
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::AddTextureBarrier( const D3D12_TEXTURE_BARRIER & aBarrier ) {
    if ( myPendingTextureBarriers.full() )
      FlushBarriers();
    myPendingTextureBarriers.push_back( aBarrier );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::AddBufferBarrier( const D3D12_BUFFER_BARRIER & aBarrier ) {
    if ( myPendingBufferBarriers.full() )
      FlushBarriers();
    myPendingBufferBarriers.push_back( aBarrier );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::AddBarrier( const D3D12_RESOURCE_BARRIER & aBarrier ) {
    ASSERT( aBarrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION );
    ASSERT( aBarrier.Transition.pResource != nullptr );

    const D3D12_RESOURCE_DESC resourceDesc = aBarrier.Transition.pResource->GetDesc();
    const uint                beforeState = ( uint ) aBarrier.Transition.StateBefore;
    const uint                afterState = ( uint ) aBarrier.Transition.StateAfter;

    if ( resourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER ) {
      TextureBarrierParams beforeParams = GetTextureBarrierParams( beforeState );
      TextureBarrierParams afterParams = GetTextureBarrierParams( afterState );

      D3D12_TEXTURE_BARRIER barrier = {};
      barrier.SyncBefore = beforeParams.mySync;
      barrier.AccessBefore = beforeParams.myAccess;
      barrier.LayoutBefore = beforeParams.myLayout;
      barrier.SyncAfter = afterParams.mySync;
      barrier.AccessAfter = afterParams.myAccess;
      barrier.LayoutAfter = afterParams.myLayout;
      barrier.pResource = aBarrier.Transition.pResource;
      barrier.Subresources = locResolveAllBarrierSubresourceRange();
      barrier.Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE;
      AddTextureBarrier( barrier );
    } else {
      BufferBarrierParams beforeParams = GetBufferBarrierParams( beforeState );
      BufferBarrierParams afterParams = GetBufferBarrierParams( afterState );

      D3D12_BUFFER_BARRIER barrier = {};
      barrier.SyncBefore = beforeParams.mySync;
      barrier.AccessBefore = beforeParams.myAccess;
      barrier.SyncAfter = afterParams.mySync;
      barrier.AccessAfter = afterParams.myAccess;
      barrier.pResource = aBarrier.Transition.pResource;
      barrier.Offset = 0u;
      barrier.Size = UINT64_MAX;
      AddBufferBarrier( barrier );
    }
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::BindLocalBuffer( const GpuBuffer * aBuffer, const GpuBufferViewProperties & someViewProperties,
                                         uint aRegisterIndex ) {
    const RootSignatureDX12 * rootSignature = RenderCore::GetPlatformDX12()->GetRootSignature();
    const GpuBufferDX12 *     bufferDx12 = static_cast< const GpuBufferDX12 * >( aBuffer );
    const uint64              bufferViewGpuAddress = bufferDx12->GetDeviceAddress() + someViewProperties.myOffset;

    if ( someViewProperties.myIsShaderWritable ) {
      ASSERT( aRegisterIndex < rootSignature->myNumLocalRWBuffers );
      ASSERT( someViewProperties.myIsRaw || someViewProperties.myIsStructured,
              "D3D12 only supports raw or structured buffer SRVs/UAVs as root descriptor" );
      TrackBufferTransition( aBuffer, GPU_BUFFER_STATE_SHADER_WRITE );

      if ( aRegisterIndex >= myLocalRWBuffersToBind.size() )
        myLocalRWBuffersToBind.resize( aRegisterIndex + 1, UINT64_MAX );

      myLocalRWBuffersToBind[ aRegisterIndex ] = bufferViewGpuAddress;
    } else if ( someViewProperties.myIsConstantBuffer ) {
      ASSERT( aRegisterIndex < rootSignature->myNumLocalCBuffers );
      TrackBufferTransition( aBuffer, GPU_BUFFER_STATE_CONSTANT_BUFFER );

      if ( aRegisterIndex >= myLocalCBuffersToBind.size() )
        myLocalCBuffersToBind.resize( aRegisterIndex + 1, UINT64_MAX );

      myLocalCBuffersToBind[ aRegisterIndex ] = bufferViewGpuAddress;
    } else {
      ASSERT( aRegisterIndex < rootSignature->myNumLocalBuffers );
      ASSERT( someViewProperties.myIsRaw || someViewProperties.myIsStructured,
              "D3D12 only supports raw or structured buffer SRVs/UAVs as root descriptor" );
      TrackBufferTransition( aBuffer, GPU_BUFFER_STATE_SHADER_READ_ALL );

      if ( aRegisterIndex >= myLocalBuffersToBind.size() )
        myLocalBuffersToBind.resize( aRegisterIndex + 1, UINT64_MAX );

      myLocalBuffersToBind[ aRegisterIndex ] = bufferViewGpuAddress;
    }
  }
  //---------------------------------------------------------------------------//
  GpuQuery CommandListDX12::BeginQuery( GpuQueryType aType ) {
    ASSERT( aType != GpuQueryType::TIMESTAMP, "Timestamp-queries should be used with InsertTimestamp" );

    const GpuQuery query = AllocateQuery( aType );
    GpuQueryHeap * heap = RenderCore::GetQueryHeap( aType );

    const GpuQueryHeapDX12 * queryHeapDx12 = static_cast< const GpuQueryHeapDX12 * >( heap );
    const D3D12_QUERY_TYPE   queryTypeDx12 = Adapter::ResolveQueryType( aType );

    myCommandList->BeginQuery( queryHeapDx12->myHeap.Get(), queryTypeDx12, query.myIndexInHeap );
    return query;
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::EndQuery( const GpuQuery & aQuery ) {
    ASSERT( aQuery.myFrame == Time::ourFrameIdx );
    ASSERT( aQuery.myType != GpuQueryType::TIMESTAMP, "Timestamp-queries should be used with InsertTimestamp" );
    ASSERT( aQuery.myIsOpen );

    aQuery.myIsOpen = false;

    const GpuQueryType queryType = aQuery.myType;
    GpuQueryHeap *     heap = RenderCore::GetQueryHeap( queryType );

    const GpuQueryHeapDX12 * queryHeapDx12 = static_cast< const GpuQueryHeapDX12 * >( heap );
    const D3D12_QUERY_TYPE   queryTypeDx12 = Adapter::ResolveQueryType( queryType );

    myCommandList->EndQuery( queryHeapDx12->myHeap.Get(), queryTypeDx12, aQuery.myIndexInHeap );
  }
  //---------------------------------------------------------------------------//
  GpuQuery CommandListDX12::InsertTimestamp() {
    const GpuQuery query = AllocateQuery( GpuQueryType::TIMESTAMP );
    query.myIsOpen = false;

    GpuQueryHeap *           heap = RenderCore::GetQueryHeap( GpuQueryType::TIMESTAMP );
    const GpuQueryHeapDX12 * queryHeapDx12 = static_cast< const GpuQueryHeapDX12 * >( heap );

    myCommandList->EndQuery( queryHeapDx12->myHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, query.myIndexInHeap );
    return query;
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::CopyQueryDataToBuffer( const GpuQueryHeap * aQueryHeap, const GpuBuffer * aBuffer,
                                               uint aFirstQueryIndex, uint aNumQueries, uint64 aBufferOffset ) {
    const GpuQueryHeapDX12 * queryHeapDx12 = static_cast< const GpuQueryHeapDX12 * >( aQueryHeap );
    const GpuBufferDX12 *    bufferDx12 = static_cast< const GpuBufferDX12 * >( aBuffer );

    TrackBufferTransition( aBuffer, GPU_BUFFER_STATE_COPY_DEST );
    FlushBarriers();

    myCommandList->ResolveQueryData( queryHeapDx12->myHeap.Get(), Adapter::ResolveQueryType( aQueryHeap->myType ),
                                     aFirstQueryIndex, aNumQueries, bufferDx12->GetDX12Data()->myResource.Get(),
                                     aBufferOffset );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::BeginMarkerRegion( const char * aName, uint aColor ) {
    CommandList::BeginMarkerRegion( aName, aColor );
    PIXBeginEvent( myCommandList, aColor, aName );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::EndMarkerRegion() {
    CommandList::EndMarkerRegion();
    PIXEndEvent( myCommandList );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::TransitionResource( const GpuResource * aResource, const SubresourceRange & aSubresourceRange,
                                            ResourceTransition aTransition, uint /* someUsageFlags = 0u*/ ) {
    switch ( aTransition ) {
      case ResourceTransition::TO_SHARED_CONTEXT_READ:
        if ( aResource->IsTexture() )
          TrackTextureSubresourceTransition( aResource, aSubresourceRange, GPU_TEXTURE_STATE_COMMON, true );
        else
          TrackBufferSubresourceTransition( aResource, aSubresourceRange, GPU_BUFFER_STATE_UNDEFINED, true );
        break;
      default:
        ASSERT( false );
    }
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::PrepareResourceShaderAccess( const GpuResource *      aResource,
                                                     const SubresourceRange & aSubresourceRange,
                                                     ShaderResourceAccess     aTransition ) {
    switch ( aTransition ) {
      case SHADER_RESOURCE_ACCESS_SRV:
        if ( aResource->IsTexture() )
          TrackTextureSubresourceTransition( aResource, aSubresourceRange, GPU_TEXTURE_STATE_SHADER_READ_ALL );
        else
          TrackBufferSubresourceTransition( aResource, aSubresourceRange, GPU_BUFFER_STATE_SHADER_READ_ALL );
        break;
      case SHADER_RESOURCE_ACCESS_RTAS:
        TrackBufferSubresourceTransition( aResource, aSubresourceRange,
                                          GPU_BUFFER_STATE_RT_ACCELERATION_STRUCTURE );
        break;
      case SHADER_RESOURCE_ACCESS_UAV:
        if ( aResource->IsTexture() )
          TrackTextureSubresourceTransition( aResource, aSubresourceRange, GPU_TEXTURE_STATE_SHADER_WRITE );
        else
          TrackBufferSubresourceTransition( aResource, aSubresourceRange, GPU_BUFFER_STATE_SHADER_WRITE );
        break;
      default:
        ASSERT( false, "Missing implementation!" );
    }
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::ResourceUAVbarrier( const GpuResource ** /* someResources */, uint /* aNumResources */ ) {
    D3D12_GLOBAL_BARRIER globalBarrier = {};
    globalBarrier.SyncBefore = D3D12_BARRIER_SYNC_ALL_SHADING |
                               D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE;
    globalBarrier.SyncAfter = D3D12_BARRIER_SYNC_ALL_SHADING |
                              D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE |
                              D3D12_BARRIER_SYNC_RAYTRACING;
    globalBarrier.AccessBefore = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS |
                                 D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE;
    globalBarrier.AccessAfter = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS |
                                D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ |
                                D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE;

    D3D12_BARRIER_GROUP barrierGroup = {};
    barrierGroup.Type = D3D12_BARRIER_TYPE_GLOBAL;
    barrierGroup.NumBarriers = 1u;
    barrierGroup.pGlobalBarriers = &globalBarrier;
    myCommandList->Barrier( 1u, &barrierGroup );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::BindVertexBuffers( const GpuBuffer ** someBuffers, uint64 * someOffsets, uint64 * someSizes,
                                           uint aNumBuffers, const VertexInputLayout * anInputLayout /*= nullptr*/ ) {
    const Shader * vertexShader =
        myGraphicsPipelineState.myShaderPipeline
            ? myGraphicsPipelineState.myShaderPipeline->GetShader( ShaderStage::SHADERSTAGE_VERTEX )
            : nullptr;
    const VertexInputLayout * shaderInputLayout =
        vertexShader ? RenderCore::GetVertexInputLayout( vertexShader->myDefaultVertexInputLayout ) : nullptr;
    const VertexInputLayout * inputLayout = anInputLayout ? anInputLayout : shaderInputLayout;

    ASSERT( inputLayout && inputLayout->myProperties.myBufferBindings.size() == aNumBuffers );

    const bool isDefaultLayout = inputLayout == shaderInputLayout;
    const bool pipelineStateIsDefault = myGraphicsPipelineState.myVertexInputLayout == nullptr;
    if ( myGraphicsPipelineState.myVertexInputLayout != inputLayout &&
         !( isDefaultLayout && pipelineStateIsDefault ) ) {
      myGraphicsPipelineState.myVertexInputLayout = inputLayout;
      myGraphicsPipelineState.myIsDirty = true;
    }

    eastl::fixed_vector< D3D12_VERTEX_BUFFER_VIEW, 4 > vertexBufferViews;
    for ( uint i = 0u; i < aNumBuffers; ++i ) {
      const GpuBuffer * buffer = someBuffers[ i ];
      TrackBufferTransition( buffer, GPU_BUFFER_STATE_VERTEX_INDEX );

      const GpuResourceDataDX12 * resourceDataDx12 = buffer->GetDX12Data();
      const uint64                resourceStartAddress = resourceDataDx12->myResource->GetGPUVirtualAddress();

      D3D12_VERTEX_BUFFER_VIEW & vertexBufferView = vertexBufferViews.push_back();
      vertexBufferView.BufferLocation = resourceStartAddress + someOffsets[ i ];
      ASSERT( someSizes[ i ] <= UINT_MAX );
      vertexBufferView.SizeInBytes = static_cast< uint >( someSizes[ i ] );

      ASSERT( inputLayout->myProperties.myBufferBindings[ i ].myStride <= vertexBufferView.SizeInBytes );
      vertexBufferView.StrideInBytes = inputLayout->myProperties.myBufferBindings[ i ].myStride;
    }

    myCommandList->IASetVertexBuffers( 0, ( uint ) vertexBufferViews.size(), vertexBufferViews.data() );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::BindIndexBuffer( const GpuBuffer * aBuffer, uint anIndexSize, uint64 anIndexOffset /* = 0u */,
                                         uint64 aNumIndices /* =~0ULL*/ ) {
    ASSERT( anIndexSize == 2u || anIndexSize == 4u );

    D3D12_INDEX_BUFFER_VIEW indexBufferView;

    const GpuResourceDataDX12 * storage = aBuffer->GetDX12Data();

    uint64 resourceStartAddress = storage->myResource->GetGPUVirtualAddress();
    indexBufferView.BufferLocation = resourceStartAddress + anIndexOffset;

    indexBufferView.Format = anIndexSize == 2u ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

    const uint64 byteSize = glm::min( aNumIndices, aBuffer->GetByteSize() );

    ASSERT( byteSize <= UINT_MAX );
    indexBufferView.SizeInBytes = static_cast< uint >( byteSize );

    TrackBufferTransition( aBuffer, GPU_BUFFER_STATE_VERTEX_INDEX );

    myCommandList->IASetIndexBuffer( &indexBufferView );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::DrawInstanced( uint aNumVerticesPerInstance, uint aNumInstances, uint aBaseVertex,
                                       uint aStartInstance ) {
    CommandList::DrawInstanced( aNumVerticesPerInstance, aNumInstances, aBaseVertex, aStartInstance );

    ApplyViewportAndClipRect();
    ApplyRenderTargets();
    ApplyTopologyType();
    ApplyGraphicsPipelineState();
    FlushBarriers();
    ApplyResourceBindings();

    myCommandList->DrawInstanced( aNumVerticesPerInstance, aNumInstances, aBaseVertex, aStartInstance );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::DrawIndexedInstanced( uint aNumIndicesPerInstance, uint aNumInstances, uint aStartIndex,
                                              uint aBaseVertex, uint aStartInstance ) {
    CommandList::DrawIndexedInstanced( aNumIndicesPerInstance, aNumInstances, aStartIndex, aBaseVertex,
                                       aStartInstance );

    ApplyViewportAndClipRect();
    ApplyRenderTargets();
    ApplyTopologyType();
    ApplyGraphicsPipelineState();
    FlushBarriers();
    ApplyResourceBindings();

    myCommandList->DrawIndexedInstanced( aNumIndicesPerInstance, aNumInstances, aStartIndex, aBaseVertex,
                                         aStartInstance );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::ApplyViewportAndClipRect() {
    if ( myViewportDirty ) {
      D3D12_VIEWPORT viewport = { 0u };
      viewport.TopLeftX = static_cast< float >( myViewportParams.x );
      viewport.TopLeftY = static_cast< float >( myViewportParams.y );
      viewport.Width = static_cast< float >( myViewportParams.z );
      viewport.Height = static_cast< float >( myViewportParams.w );
      viewport.MinDepth = 0.0f;
      viewport.MaxDepth = 1.0f;

      myCommandList->RSSetViewports( 1u, &viewport );

      myClipRectDirty = true;
      myViewportDirty = false;
    }

    if ( myClipRectDirty ) {
      D3D12_RECT rect = { 0u };
      rect.left = myClipRect.x;
      rect.top = myClipRect.y;
      rect.right = myClipRect.z;
      rect.bottom = myClipRect.w;

      myCommandList->RSSetScissorRects( 1u, &rect );

      myClipRectDirty = false;
    }
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::ApplyRenderTargets() {
    const uint                  numRtsToSet = myGraphicsPipelineState.myNumRenderTargets;
    D3D12_CPU_DESCRIPTOR_HANDLE rtDescriptors[ RenderConstants::kMaxNumRenderTargets ];

    for ( uint i = 0u; i < numRtsToSet; ++i ) {
      ASSERT( myRenderTargets[ i ] != nullptr );

      const GpuResourceViewDataDX12 & viewData = myRenderTargets[ i ]->myDX12Data;
      ASSERT( myRenderTargets[ i ]->myType == GpuResourceViewType::RTV );

      TrackTextureSubresourceTransition( myRenderTargets[ i ]->GetResource(),
                                         myRenderTargets[ i ]->GetSubresourceRange(),
                                         GPU_TEXTURE_STATE_RENDER_TARGET );

      rtDescriptors[ i ] = viewData.myDescriptor.myCpuHandle;
    }

    if ( myDepthStencilTarget != nullptr ) {
      const GpuResourceViewDataDX12 & dsvViewData = myDepthStencilTarget->myDX12Data;
      ASSERT( myDepthStencilTarget->myType == GpuResourceViewType::DSV );

      const uint depthState = myDepthStencilTarget->GetProperties().myIsDepthReadOnly
                                  ? GPU_TEXTURE_STATE_DEPTH_READ
                                  : GPU_TEXTURE_STATE_DEPTH_WRITE;
      TrackTextureSubresourceTransition( myDepthStencilTarget->GetResource(),
                                         myDepthStencilTarget->GetSubresourceRange(), depthState );

      if ( myRenderTargetsDirty )
        myCommandList->OMSetRenderTargets( numRtsToSet, rtDescriptors, false, &dsvViewData.myDescriptor.myCpuHandle );
    } else {
      if ( myRenderTargetsDirty )
        myCommandList->OMSetRenderTargets( numRtsToSet, rtDescriptors, false, nullptr );
    }

    myRenderTargetsDirty = false;
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::ApplyTopologyType() {
    if ( !myTopologyDirty )
      return;

    myTopologyDirty = false;
    myCommandList->IASetPrimitiveTopology( Adapter::ResolveTopology( myGraphicsPipelineState.myTopologyType ) );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::ApplyResourceBindings() {
    RenderCore_PlatformDX12 * platformDx12 = RenderCore::GetPlatformDX12();
    const RootSignatureDX12 * rootSignature = platformDx12->GetRootSignature();

    if ( myLocalBuffersToBind.empty() && myLocalRWBuffersToBind.empty() && myLocalCBuffersToBind.empty() )
      return;

    uint rootParamIdx = rootSignature->myRootParamIndex_LocalBuffers;
    for ( uint i = 0; i < ( uint ) myLocalBuffersToBind.size(); ++i ) {
      if ( myLocalBuffersToBind[ i ] == UINT64_MAX )
        continue;

      if ( myCommandListType == CommandListType::Graphics || myCommandListType == CommandListType::Compute )
        myCommandList->SetComputeRootShaderResourceView( rootParamIdx + i, myLocalBuffersToBind[ i ] );
      if ( myCommandListType == CommandListType::Graphics )
        myCommandList->SetGraphicsRootShaderResourceView( rootParamIdx + i, myLocalBuffersToBind[ i ] );
    }
    myLocalBuffersToBind.clear();

    rootParamIdx = rootSignature->myRootParamIndex_LocalRWBuffers;
    for ( uint i = 0; i < ( uint ) myLocalRWBuffersToBind.size(); ++i ) {
      if ( myLocalRWBuffersToBind[ i ] == UINT64_MAX )
        continue;

      if ( myCommandListType == CommandListType::Graphics || myCommandListType == CommandListType::Compute )
        myCommandList->SetComputeRootUnorderedAccessView( rootParamIdx + i, myLocalRWBuffersToBind[ i ] );
      if ( myCommandListType == CommandListType::Graphics )
        myCommandList->SetGraphicsRootUnorderedAccessView( rootParamIdx + i, myLocalRWBuffersToBind[ i ] );
    }
    myLocalRWBuffersToBind.clear();

    rootParamIdx = rootSignature->myRootParamIndex_LocalCBuffers;
    for ( uint i = 0; i < ( uint ) myLocalCBuffersToBind.size(); ++i ) {
      if ( myLocalCBuffersToBind[ i ] == UINT64_MAX )
        continue;

      if ( myCommandListType == CommandListType::Graphics || myCommandListType == CommandListType::Compute )
        myCommandList->SetComputeRootConstantBufferView( rootParamIdx + i, myLocalCBuffersToBind[ i ] );
      if ( myCommandListType == CommandListType::Graphics )
        myCommandList->SetGraphicsRootConstantBufferView( rootParamIdx + i, myLocalCBuffersToBind[ i ] );
    }
    myLocalCBuffersToBind.clear();
  }
  //---------------------------------------------------------------------------//
  bool CommandListDX12::ValidateSubresourceTransition( const GpuResource * aResource, uint aSubresourceIndex,
                                                        uint aDstState, bool aIsTexture, bool & aIsWAWOut ) {
    aIsWAWOut = false;
    const GpuResourceBarrierDataDX12 & globalData = aResource->GetDX12Data()->myHazardData;
    if ( !globalData.myCanChangeStates )
      return false;

    uint            currState = globalData.mySubresources[ aSubresourceIndex ].myState;
    CommandListType currContext = globalData.mySubresources[ aSubresourceIndex ].myContext;

    eastl::fixed_hash_map< const GpuResource *, LocalHazardData, kNumExpectedResourcesPerDispatch >::iterator it =
        myLocalHazardData.find( aResource );
    const bool hasLocalData = it != myLocalHazardData.end();
    if ( hasLocalData )
      currState = it->second.mySubresources[ aSubresourceIndex ].myState;

    const bool dstIsWrite = aIsTexture ? IsTextureWriteState( aDstState ) : IsBufferWriteState( aDstState );

    if ( dstIsWrite && currState == aDstState && hasLocalData ) {
      aIsWAWOut = true;
      return true;
    }

    if ( !dstIsWrite ) {
      const bool currHasAllDstBits = ( currState & aDstState ) == aDstState;
      const bool inSharedRead = !hasLocalData && currContext == CommandListType::SHARED_READ;
      if ( currHasAllDstBits && ( hasLocalData || inSharedRead ) )
        return false;
      if ( inSharedRead && !currHasAllDstBits ) {
        ASSERT( false, "Cannot transition from SHARED_READ state; resource needs prior explicit transition" );
        return false;
      }
    }

    return true;
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::TrackTextureSubresourceTransition( const GpuResource *      aResource,
                                                           const SubresourceRange & aSubresourceRange, uint aDstState,
                                                           bool aToSharedReadState /* = false */ ) {
    if ( aResource == nullptr )
      return;
    ASSERT( aResource->IsTexture() );

    const GpuResourceBarrierDataDX12 & hazardData = aResource->GetDX12Data()->myHazardData;
    if ( !hazardData.myCanChangeStates && !aToSharedReadState )
      return;

    if ( myCommandListType == CommandListType::Compute ) {
      ASSERT( ( aDstState & ( GPU_TEXTURE_STATE_RENDER_TARGET | GPU_TEXTURE_STATE_DEPTH_WRITE |
                              GPU_TEXTURE_STATE_DEPTH_READ ) ) == 0u,
              "Texture state not supported on compute queue" );
    } else if ( myCommandListType == CommandListType::DMA ) {
      ASSERT( ( aDstState & ~( GPU_TEXTURE_STATE_COPY_SOURCE | GPU_TEXTURE_STATE_COPY_DEST ) ) == 0u,
              "DMA queue only supports COPY_SOURCE/COPY_DEST for textures" );
    }

    TextureBarrierParams dstParams = GetTextureBarrierParams( aDstState );

    const uint                    numSubresources = aSubresourceRange.GetNumSubresources();
    eastl::fixed_vector< bool, 64 > canTransition( numSubresources, false );
    eastl::fixed_vector< bool, 64 > isWAW( numSubresources, false );
    uint                          numPossibleTransitions = 0u;
    uint                          i = 0u;
    for ( SubresourceIterator it = aSubresourceRange.Begin(); it != aSubresourceRange.End(); ++it, ++i ) {
      bool waw = false;
      bool ok = ValidateSubresourceTransition( aResource, aResource->GetSubresourceIndex( *it ), aDstState, true, waw );
      canTransition[ i ] = ok;
      isWAW[ i ] = waw;
      if ( ok )
        ++numPossibleTransitions;
    }

    if ( numPossibleTransitions == 0u && !aToSharedReadState )
      return;

    LocalHazardData * localData = nullptr;
    {
      eastl::fixed_hash_map< const GpuResource *, LocalHazardData, kNumExpectedResourcesPerDispatch >::iterator it =
          myLocalHazardData.find( aResource );
      if ( it == myLocalHazardData.end() ) {
        localData = &myLocalHazardData[ aResource ];
        localData->mySubresources.resize( aResource->mySubresources.GetNumSubresources() );
      } else {
        localData = &it->second;
      }
    }

    bool canUseAllSubresources = ( numPossibleTransitions == ( uint ) localData->mySubresources.size() );
    if ( canUseAllSubresources ) {
      const uint firstSrcState = localData->mySubresources[ 0 ].myState;
      for ( uint sub = 0u; canUseAllSubresources && sub < ( uint ) localData->mySubresources.size(); ++sub ) {
        canUseAllSubresources = localData->mySubresources[ sub ].myWasUsed &&
                                localData->mySubresources[ sub ].myState == firstSrcState;
      }
    }

    if ( canUseAllSubresources ) {
      TextureBarrierParams beforeParams = GetTextureBarrierParams( localData->mySubresources[ 0 ].myState );
      D3D12_TEXTURE_BARRIER barrier = {};
      barrier.SyncBefore = isWAW[ 0 ] ? dstParams.mySync : beforeParams.mySync;
      barrier.AccessBefore = isWAW[ 0 ] ? dstParams.myAccess : beforeParams.myAccess;
      barrier.LayoutBefore = isWAW[ 0 ] ? dstParams.myLayout : beforeParams.myLayout;
      barrier.SyncAfter = dstParams.mySync;
      barrier.AccessAfter = dstParams.myAccess;
      barrier.LayoutAfter = dstParams.myLayout;
      barrier.pResource = aResource->GetDX12Data()->myResource.Get();
      barrier.Subresources = locResolveBarrierSubresourceRange( aResource->GetSubresources() );
      barrier.Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE;
      AddTextureBarrier( barrier );
    } else {
      i = 0u;
      for ( SubresourceIterator it = aSubresourceRange.Begin(); it != aSubresourceRange.End(); ++it, ++i ) {
        if ( !canTransition[ i ] )
          continue;

        const uint subIdx = aResource->GetSubresourceIndex( *it );
        SubresourceHazardData & subData = localData->mySubresources[ subIdx ];
        if ( !subData.myWasUsed )
          continue;

        TextureBarrierParams beforeParams = GetTextureBarrierParams( subData.myState );
        D3D12_TEXTURE_BARRIER barrier = {};
        barrier.SyncBefore = isWAW[ i ] ? dstParams.mySync : beforeParams.mySync;
        barrier.AccessBefore = isWAW[ i ] ? dstParams.myAccess : beforeParams.myAccess;
        barrier.LayoutBefore = isWAW[ i ] ? dstParams.myLayout : beforeParams.myLayout;
        barrier.SyncAfter = dstParams.mySync;
        barrier.AccessAfter = dstParams.myAccess;
        barrier.LayoutAfter = dstParams.myLayout;
        barrier.pResource = aResource->GetDX12Data()->myResource.Get();
        barrier.Subresources = locResolveBarrierSubresourceRange( aResource->GetSubresourceLocation( subIdx ) );
        barrier.Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE;
        AddTextureBarrier( barrier );
      }
    }

    i = 0u;
    for ( SubresourceIterator it = aSubresourceRange.Begin(); it != aSubresourceRange.End(); ++it, ++i ) {
      const uint              subIdx = aResource->GetSubresourceIndex( *it );
      SubresourceHazardData & subData = localData->mySubresources[ subIdx ];

      if ( aToSharedReadState ) {
        subData.myWasWritten = false;
        subData.myIsSharedReadState = true;
      }

      if ( !canTransition[ i ] )
        continue;

      if ( !subData.myWasUsed )
        subData.myFirstDstState = aDstState;

      subData.myWasUsed = true;
      subData.myState = aDstState;
      if ( IsTextureWriteState( aDstState ) ) {
        subData.myWasWritten = true;
        subData.myIsSharedReadState = false;
      }
    }
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::TrackTextureTransition( const GpuResource * aResource, uint aNewState,
                                                bool aToSharedReadState /* = false */ ) {
    TrackTextureSubresourceTransition( aResource, aResource->GetSubresources(), aNewState, aToSharedReadState );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::TrackBufferSubresourceTransition( const GpuResource *      aResource,
                                                          const SubresourceRange & aSubresourceRange, uint aDstState,
                                                          bool aToSharedReadState /* = false */ ) {
    if ( aResource == nullptr )
      return;
    ASSERT( !aResource->IsTexture() );

    const GpuResourceBarrierDataDX12 & hazardData = aResource->GetDX12Data()->myHazardData;
    if ( !hazardData.myCanChangeStates && !aToSharedReadState )
      return;

    if ( myCommandListType == CommandListType::DMA ) {
      ASSERT( ( aDstState & ~( GPU_BUFFER_STATE_COPY_SOURCE | GPU_BUFFER_STATE_COPY_DEST ) ) == 0u,
              "DMA queue only supports COPY_SOURCE/COPY_DEST for buffers" );
    }

    BufferBarrierParams dstParams = GetBufferBarrierParams( aDstState );

    const uint                    numSubresources = aSubresourceRange.GetNumSubresources();
    eastl::fixed_vector< bool, 64 > canTransition( numSubresources, false );
    eastl::fixed_vector< bool, 64 > isWAW( numSubresources, false );
    uint                          numPossibleTransitions = 0u;
    uint                          i = 0u;
    for ( SubresourceIterator it = aSubresourceRange.Begin(); it != aSubresourceRange.End(); ++it, ++i ) {
      bool waw = false;
      bool ok = ValidateSubresourceTransition( aResource, aResource->GetSubresourceIndex( *it ), aDstState, false, waw );
      canTransition[ i ] = ok;
      isWAW[ i ] = waw;
      if ( ok )
        ++numPossibleTransitions;
    }

    if ( numPossibleTransitions == 0u && !aToSharedReadState )
      return;

    LocalHazardData * localData = nullptr;
    {
      eastl::fixed_hash_map< const GpuResource *, LocalHazardData, kNumExpectedResourcesPerDispatch >::iterator it =
          myLocalHazardData.find( aResource );
      if ( it == myLocalHazardData.end() ) {
        localData = &myLocalHazardData[ aResource ];
        localData->mySubresources.resize( aResource->mySubresources.GetNumSubresources() );
      } else {
        localData = &it->second;
      }
    }

    i = 0u;
    for ( SubresourceIterator it = aSubresourceRange.Begin(); it != aSubresourceRange.End(); ++it, ++i ) {
      if ( !canTransition[ i ] )
        continue;

      const uint subIdx = aResource->GetSubresourceIndex( *it );
      SubresourceHazardData & subData = localData->mySubresources[ subIdx ];
      if ( !subData.myWasUsed )
        continue;

      BufferBarrierParams beforeParams = GetBufferBarrierParams( subData.myState );
      D3D12_BUFFER_BARRIER barrier = {};
      barrier.SyncBefore = isWAW[ i ] ? dstParams.mySync : beforeParams.mySync;
      barrier.AccessBefore = isWAW[ i ] ? dstParams.myAccess : beforeParams.myAccess;
      barrier.SyncAfter = dstParams.mySync;
      barrier.AccessAfter = dstParams.myAccess;
      barrier.pResource = aResource->GetDX12Data()->myResource.Get();
      barrier.Offset = 0;
      barrier.Size = UINT64_MAX;
      AddBufferBarrier( barrier );
    }

    i = 0u;
    for ( SubresourceIterator it = aSubresourceRange.Begin(); it != aSubresourceRange.End(); ++it, ++i ) {
      const uint              subIdx = aResource->GetSubresourceIndex( *it );
      SubresourceHazardData & subData = localData->mySubresources[ subIdx ];

      if ( aToSharedReadState ) {
        subData.myWasWritten = false;
        subData.myIsSharedReadState = true;
      }

      if ( !canTransition[ i ] )
        continue;

      if ( !subData.myWasUsed )
        subData.myFirstDstState = aDstState;

      subData.myWasUsed = true;
      subData.myState = aDstState;
      if ( IsBufferWriteState( aDstState ) ) {
        subData.myWasWritten = true;
        subData.myIsSharedReadState = false;
      }
    }
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::TrackBufferTransition( const GpuResource * aResource, uint aNewState,
                                               bool aToSharedReadState /* = false */ ) {
    TrackBufferSubresourceTransition( aResource, aResource->GetSubresources(), aNewState, aToSharedReadState );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::ApplyGraphicsPipelineState() {
    if ( !myGraphicsPipelineState.myIsDirty )
      return;

    myGraphicsPipelineState.myIsDirty = false;

    ID3D12PipelineState * pso =
        RenderCore::GetPlatformDX12()->GetPipelineStateCache().GetCreateGraphicsPSO( myGraphicsPipelineState );
    myCommandList->SetPipelineState( pso );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::ApplyComputePipelineState() {
    if ( !myComputePipelineState.myIsDirty )
      return;

    myComputePipelineState.myIsDirty = false;

    ID3D12PipelineState * pso =
        RenderCore::GetPlatformDX12()->GetPipelineStateCache().GetCreateComputePSO( myComputePipelineState );
    myCommandList->SetPipelineState( pso );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::ApplyRaytracingPipelineState() {
    if ( !myRaytracingPipelineStateDirty )
      return;

    myRaytracingPipelineStateDirty = false;

    RtPipelineStateDX12 * rtPsoDx12 = static_cast< RtPipelineStateDX12 * >( myRaytracingPipelineState );
    myCommandList->SetPipelineState1( rtPsoDx12->myStateObject.Get() );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::Dispatch( const glm::int3 & aNumThreads ) {
    ApplyComputePipelineState();
    ApplyResourceBindings();
    FlushBarriers();
    ASSERT( myComputePipelineState.myShaderPipeline != nullptr );

    const Shader * shader = myComputePipelineState.myShaderPipeline->GetShader( ShaderStage::SHADERSTAGE_COMPUTE );
    ASSERT( shader != nullptr );

    const glm::int3 & numGroupThreads = shader->GetProperties().myNumGroupThreads;
    const glm::int3   numGroups = glm::max( glm::int3( 1 ), aNumThreads / numGroupThreads );
    myCommandList->Dispatch( static_cast< uint >( numGroups.x ), static_cast< uint >( numGroups.y ),
                             static_cast< uint >( numGroups.z ) );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::DispatchRays( const DispatchRaysDesc & aDesc ) {
    ASSERT( aDesc.myRayGenShaderTableRange.mySbtBuffer != nullptr );

    ApplyRaytracingPipelineState();
    ASSERT( myRaytracingPipelineState != nullptr );

    TrackBufferTransition( aDesc.myRayGenShaderTableRange.mySbtBuffer, GPU_BUFFER_STATE_RT_SBT );
    TrackBufferTransition( aDesc.myCallableShaderTableRange.mySbtBuffer, GPU_BUFFER_STATE_RT_SBT );
    TrackBufferTransition( aDesc.myMissShaderTableRange.mySbtBuffer, GPU_BUFFER_STATE_RT_SBT );
    TrackBufferTransition( aDesc.myHitGroupTableRange.mySbtBuffer, GPU_BUFFER_STATE_RT_SBT );

    ApplyResourceBindings();
    FlushBarriers();

    D3D12_DISPATCH_RAYS_DESC desc = {};
    desc.Width = aDesc.myWidth;
    desc.Height = aDesc.myHeight;
    desc.Depth = aDesc.myDepth;

    desc.RayGenerationShaderRecord.StartAddress =
        aDesc.myRayGenShaderTableRange.mySbtBuffer->GetDeviceAddress() + aDesc.myRayGenShaderTableRange.myOffset;
    desc.RayGenerationShaderRecord.SizeInBytes = aDesc.myRayGenShaderTableRange.mySize;

    if ( aDesc.myMissShaderTableRange.mySbtBuffer != nullptr ) {
      desc.MissShaderTable.StartAddress =
          aDesc.myMissShaderTableRange.mySbtBuffer->GetDeviceAddress() + aDesc.myMissShaderTableRange.myOffset;
      desc.MissShaderTable.SizeInBytes = aDesc.myMissShaderTableRange.mySize;
      desc.MissShaderTable.StrideInBytes = aDesc.myMissShaderTableRange.myStride;
    }

    if ( aDesc.myHitGroupTableRange.mySbtBuffer != nullptr ) {
      desc.HitGroupTable.StartAddress =
          aDesc.myHitGroupTableRange.mySbtBuffer->GetDeviceAddress() + aDesc.myHitGroupTableRange.myOffset;
      desc.HitGroupTable.SizeInBytes = aDesc.myHitGroupTableRange.mySize;
      desc.HitGroupTable.StrideInBytes = aDesc.myHitGroupTableRange.myStride;
    }

    if ( aDesc.myCallableShaderTableRange.mySbtBuffer != nullptr ) {
      desc.CallableShaderTable.StartAddress =
          aDesc.myCallableShaderTableRange.mySbtBuffer->GetDeviceAddress() + aDesc.myCallableShaderTableRange.myOffset;
      desc.CallableShaderTable.SizeInBytes = aDesc.myCallableShaderTableRange.mySize;
      desc.CallableShaderTable.StrideInBytes = aDesc.myCallableShaderTableRange.myStride;
    }

    myCommandList->DispatchRays( &desc );
  }
  //---------------------------------------------------------------------------//
  void CommandListDX12::Close() {
    if ( myIsOpen )
      ASSERT_HRESULT( myCommandList->Close() );

    myIsOpen = false;
  }
  //---------------------------------------------------------------------------//
  //---------------------------------------------------------------------------//

  //---------------------------------------------------------------------------//
}  // namespace Fancy

#endif
