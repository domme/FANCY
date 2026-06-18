#include "fancy_core_precompile.h"
#include "GpuBufferDX12.h"

#include "Common/StringUtil.h"

#include "Rendering/RenderCore.h"
#include "Rendering/CommandList.h"

#include "RenderCore_PlatformDX12.h"
#include "GpuResourceDataDX12.h"
#include "GpuResourceViewDataDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
  //---------------------------------------------------------------------------//
  GpuBufferDX12::~GpuBufferDX12() {
    Destroy();
  }
  //---------------------------------------------------------------------------//
  bool GpuBufferDX12::IsValid() const {
    return GetDX12Data() != nullptr && GetDX12Data()->myResource.Get() != nullptr;
  }
  //---------------------------------------------------------------------------//
  void GpuBufferDX12::SetName( const char * aName ) {
    GpuBuffer::SetName( aName );

    if ( GpuResourceDataDX12 * dataDx12 = GetDX12Data() ) {
      eastl::wstring wName = StringUtil::ToWideString( myName );
      dataDx12->myResource->SetName( wName.c_str() );
    }
  }
  //---------------------------------------------------------------------------//
  void GpuBufferDX12::Create( const GpuBufferProperties & someProperties, const char * aName /*= nullptr*/,
                              const void * pInitialData /*= nullptr*/ ) {
    ASSERT( someProperties.myElementSizeBytes > 0 && someProperties.myNumElements > 0,
            "Invalid buffer size specified" );

    Destroy();
    GpuResourceDataDX12 dataDx12;

    myProperties = someProperties;
    myName = aName != nullptr ? aName : "GpuBuffer_Unnamed";

    myAlignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    if ( ( someProperties.myBindFlags & ( uint ) GpuBufferBindFlags::CONSTANT_BUFFER ) != 0u )
      myAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;

    const uint64 pitch =
        MathUtil::Align( someProperties.myNumElements * someProperties.myElementSizeBytes, myAlignment );

    D3D12_RESOURCE_DESC resourceDesc;
    memset( &resourceDesc, 0, sizeof( resourceDesc ) );
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = pitch;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    // For enhanced barriers: RTAS resources need both the RTAS flag and UAV flag
    if ( someProperties.myBindFlags & ( uint ) GpuBufferBindFlags::RT_ACCELERATION_STRUCTURE_STORAGE )
      resourceDesc.Flags = D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    else if ( someProperties.myIsShaderWritable )
      resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    else
      resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    // Determine initial buffer state based on properties
    uint initialBufferState = GPU_BUFFER_STATE_SHADER_READ_NON_PIXEL;
    bool canChangeStates    = true;

    if ( someProperties.myCpuAccess == CpuMemoryAccessType::CPU_WRITE ) {
      canChangeStates    = false;
      initialBufferState = GPU_BUFFER_STATE_COPY_SOURCE;   // Upload heap: always GENERIC_READ equivalent
    } else if ( someProperties.myCpuAccess == CpuMemoryAccessType::CPU_READ ) {
      canChangeStates    = false;
      initialBufferState = GPU_BUFFER_STATE_COPY_DEST;
    } else if ( someProperties.myBindFlags & ( uint ) GpuBufferBindFlags::RT_ACCELERATION_STRUCTURE_STORAGE ) {
      initialBufferState = GPU_BUFFER_STATE_RT_ACCELERATION_STRUCTURE;
    } else if ( someProperties.myIsShaderWritable ) {
      initialBufferState = GPU_BUFFER_STATE_SHADER_WRITE;
    } else if ( someProperties.myBindFlags & ( uint ) GpuBufferBindFlags::CONSTANT_BUFFER ) {
      initialBufferState = GPU_BUFFER_STATE_CONSTANT_BUFFER;
    } else if ( someProperties.myBindFlags & ( ( uint ) GpuBufferBindFlags::VERTEX_BUFFER |
                                               ( uint ) GpuBufferBindFlags::INDEX_BUFFER ) ) {
      initialBufferState = GPU_BUFFER_STATE_VERTEX_INDEX;
    } else if ( someProperties.myBindFlags & ( uint ) GpuBufferBindFlags::SHADER_BUFFER ) {
      initialBufferState = GPU_BUFFER_STATE_SHADER_READ_ALL;
    } else if ( someProperties.myBindFlags & ( uint ) GpuBufferBindFlags::RT_SHADER_BINDING_TABLE ) {
      initialBufferState = GPU_BUFFER_STATE_RT_SBT;
    }

    GpuSubresourceBarrierStateDX12 subState;
    subState.myState   = initialBufferState;
    subState.myContext = CommandListType::Graphics;

    GpuResourceBarrierDataDX12 * hazardData = &dataDx12.myHazardData;
    *hazardData = GpuResourceBarrierDataDX12();
    hazardData->myCanChangeStates = canChangeStates;
    hazardData->mySubresources.push_back( subState );

    mySubresources = SubresourceRange( 0u, 1u, 0u, 1u, 0u, 1u );

    RenderCore_PlatformDX12 * dx12Platform = RenderCore::GetPlatformDX12();
    ID3D12Device12 *          device12 = dx12Platform->GetDevice();

    GpuMemoryAllocationDX12 gpuMemory = dx12Platform->AllocateGpuMemory(
        GpuMemoryType::BUFFER, someProperties.myCpuAccess, pitch, myAlignment, myName.c_str() );
    ASSERT( gpuMemory.myHeap != nullptr );

    const uint64 alignedHeapOffset = MathUtil::Align( gpuMemory.myOffsetInHeap, myAlignment );
    
    // Use CreatePlacedResource2 with enhanced barrier layout (no legacy D3D12_RESOURCE_STATES)
    // Initialize D3D12_RESOURCE_DESC1 with the resource description
    D3D12_RESOURCE_DESC1 resourceDesc1 = {};
    resourceDesc1.Dimension = resourceDesc.Dimension;
    resourceDesc1.Alignment = resourceDesc.Alignment;
    resourceDesc1.Width = resourceDesc.Width;
    resourceDesc1.Height = resourceDesc.Height;
    resourceDesc1.DepthOrArraySize = resourceDesc.DepthOrArraySize;
    resourceDesc1.MipLevels = resourceDesc.MipLevels;
    resourceDesc1.Format = resourceDesc.Format;
    resourceDesc1.SampleDesc = resourceDesc.SampleDesc;
    resourceDesc1.Layout = resourceDesc.Layout;
    resourceDesc1.Flags = resourceDesc.Flags;
    resourceDesc1.SamplerFeedbackMipRegion = {};
    ASSERT_HRESULT( device12->CreatePlacedResource2( gpuMemory.myHeap, alignedHeapOffset, &resourceDesc1, D3D12_BARRIER_LAYOUT_UNDEFINED,
                                                      nullptr, 0, nullptr, IID_PPV_ARGS( &dataDx12.myResource ) ) );

    eastl::wstring wName = StringUtil::ToWideString( myName );
    dataDx12.myResource->SetName( wName.c_str() );
    dataDx12.myGpuMemory = gpuMemory;

    myDx12Data = dataDx12;

    if ( pInitialData != nullptr ) {
      if ( someProperties.myCpuAccess == CpuMemoryAccessType::CPU_WRITE ) {
        void * dest = Map( GpuResourceMapMode::WRITE_UNSYNCHRONIZED );
        ASSERT( dest != nullptr );
        memcpy( dest, pInitialData, someProperties.myNumElements * someProperties.myElementSizeBytes );
        Unmap( GpuResourceMapMode::WRITE_UNSYNCHRONIZED );
      } else {
        CommandList * ctx = RenderCore::BeginCommandList( CommandListType::Graphics );
        ctx->UpdateBufferData( this, 0u, pInitialData,
                               someProperties.myNumElements * someProperties.myElementSizeBytes );
        RenderCore::ExecuteAndFreeCommandList( ctx, SyncMode::BLOCKING );
      }
    }
  }
  //---------------------------------------------------------------------------//
  uint64 GpuBufferDX12::GetDeviceAddress() const {
    if ( GetDX12Data() == nullptr || GetDX12Data()->myResource == nullptr )
      return 0;

    return GetDX12Data()->myResource->GetGPUVirtualAddress();
  }
  //---------------------------------------------------------------------------//
  void GpuBufferDX12::Destroy() {
    GpuResourceDataDX12 * dataDx12 = GetDX12Data();
    if ( dataDx12 != nullptr ) {
      dataDx12->myResource.Reset();

      if ( dataDx12->myGpuMemory.myHeap != nullptr )
        RenderCore::GetPlatformDX12()->ReleaseGpuMemory( dataDx12->myGpuMemory );
    }

    myProperties = GpuBufferProperties();
  }
  //---------------------------------------------------------------------------//

  //---------------------------------------------------------------------------//
  GpuBufferViewDX12::GpuBufferViewDX12( GpuBuffer * aBuffer, const GpuBufferViewProperties & someProperties,
                                        const char * aName )
      : GpuBufferView::GpuBufferView( aBuffer, someProperties, aName ) {
    GpuResourceViewDataDX12 nativeData;

    eastl::string name = myName.empty() ? aBuffer->GetName() : myName;

    bool success = false;
    if ( someProperties.myIsConstantBuffer ) {
      name.append( "CBV" );
      ASSERT( myType == GpuResourceViewType::CBV );
      nativeData.myDescriptor =
          RenderCore::GetPlatformDX12()->AllocateDescriptor( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, name.c_str() );
      ASSERT( nativeData.myDescriptor.myCpuHandle.ptr != UINT_MAX );
      success = CreateCBVdescriptor( aBuffer, someProperties, nativeData.myDescriptor );
    } else {
      GpuBufferViewProperties rawProperties = someProperties;
      rawProperties.myIsRaw = true;
      rawProperties.myIsStructured = false;

      if ( someProperties.myIsShaderWritable ) {
        name.append( "UAV" );
        ASSERT( myType == GpuResourceViewType::UAV );
        nativeData.myDescriptor = RenderCore::GetPlatformDX12()->AllocateShaderVisibleDescriptorForGlobalResource(
            GLOBAL_RESOURCE_RWBUFFER, name.c_str() );
        success = CreateUAVdescriptor( aBuffer, rawProperties, nativeData.myDescriptor );
        myGlobalDescriptorIndex = nativeData.myDescriptor.myGlobalResourceIndex;
      } else {
        if ( someProperties.myIsRtAccelerationStructure ) {
          name.append( "SRV_RT_AS" );
          ASSERT( myType == GpuResourceViewType::SRV_RT_AS );
          nativeData.myDescriptor = RenderCore::GetPlatformDX12()->AllocateShaderVisibleDescriptorForGlobalResource(
              GLOBAL_RESOURCE_RT_ACCELERATION_STRUCTURE, name.c_str() );
        } else {
          name.append( "SRV" );
          ASSERT( myType == GpuResourceViewType::SRV );
          nativeData.myDescriptor = RenderCore::GetPlatformDX12()->AllocateShaderVisibleDescriptorForGlobalResource(
              GLOBAL_RESOURCE_BUFFER, name.c_str() );
        }

        success = CreateSRVdescriptor( aBuffer, rawProperties, nativeData.myDescriptor );
        myGlobalDescriptorIndex = nativeData.myDescriptor.myGlobalResourceIndex;
      }
    }

    ASSERT( success );

    myDX12Data = nativeData;
    mySubresourceRange = SubresourceRange( 0u, 1u, 0u, 1u, 0u, 1u );
    myCoversAllSubresources = true;
  }
  //---------------------------------------------------------------------------//
  GpuBufferViewDX12::~GpuBufferViewDX12() {
    RenderCore::GetPlatformDX12()->FreeDescriptor( myDX12Data.myDescriptor );
  }
  //---------------------------------------------------------------------------//
  bool GpuBufferViewDX12::CreateSRVdescriptor( const GpuBuffer *               aBuffer,
                                               const GpuBufferViewProperties & someProperties,
                                               const DescriptorDX12 &          aDescriptor ) {
    const GpuResourceDataDX12 * dataDx12 = aBuffer->GetDX12Data();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    memset( &srvDesc, 0u, sizeof( srvDesc ) );

    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    if ( someProperties.myIsRtAccelerationStructure ) {
      srvDesc.Format = DXGI_FORMAT_UNKNOWN;
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
      srvDesc.RaytracingAccelerationStructure.Location = aBuffer->GetDeviceAddress();
    } else if ( someProperties.myIsRaw ) {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
      srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
      srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
      srvDesc.Buffer.FirstElement = someProperties.myOffset / 4;
      ASSERT( someProperties.mySize / 4 <= UINT_MAX );
      srvDesc.Buffer.NumElements = static_cast< uint >( someProperties.mySize / 4 );
    } else if ( someProperties.myIsStructured ) {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
      srvDesc.Format = DXGI_FORMAT_UNKNOWN;
      srvDesc.Buffer.StructureByteStride = someProperties.myStructureSize;
      srvDesc.Buffer.FirstElement = someProperties.myOffset / someProperties.myStructureSize;
      ASSERT( someProperties.mySize / someProperties.myStructureSize <= UINT_MAX );
      srvDesc.Buffer.NumElements = static_cast< uint >( someProperties.mySize / someProperties.myStructureSize );
    } else {
      ASSERT( someProperties.myFormat != DataFormat::UNKNOWN, "Typed buffer-SRV needs a proper format" );
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
      const DataFormat       format = someProperties.myFormat;
      const DataFormatInfo & formatInfo = DataFormatInfo::GetFormatInfo( format );
      srvDesc.Format = RenderCore_PlatformDX12::ResolveFormat( format );
      srvDesc.Buffer.FirstElement = someProperties.myOffset / BITS_TO_BYTES( formatInfo.myBitsPerPixel );
      ASSERT( someProperties.mySize / BITS_TO_BYTES( formatInfo.myBitsPerPixel ) <= UINT_MAX );
      srvDesc.Buffer.NumElements =
          static_cast< uint >( someProperties.mySize / BITS_TO_BYTES( formatInfo.myBitsPerPixel ) );
    }

    // Resource must be nullptr in case of Rt acceleration structure according to the docs. The buffer-address is
    // already included in the srvDesc
    RenderCore::GetPlatformDX12()->GetDevice()->CreateShaderResourceView(
        someProperties.myIsRtAccelerationStructure ? nullptr : dataDx12->myResource.Get(), &srvDesc,
        aDescriptor.myCpuHandle );
    return true;
  }
  //---------------------------------------------------------------------------//
  bool GpuBufferViewDX12::CreateUAVdescriptor( const GpuBuffer *               aBuffer,
                                               const GpuBufferViewProperties & someProperties,
                                               const DescriptorDX12 &          aDescriptor ) {
    const GpuResourceDataDX12 * dataDx12 = aBuffer->GetDX12Data();

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    memset( &uavDesc, 0u, sizeof( uavDesc ) );

    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

    if ( someProperties.myIsRaw ) {
      uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
      uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
      uavDesc.Buffer.FirstElement = someProperties.myOffset / 4;
      ASSERT( someProperties.mySize / 4 <= UINT_MAX );
      uavDesc.Buffer.NumElements = static_cast< uint >( someProperties.mySize / 4 );
    } else if ( someProperties.myIsStructured ) {
      uavDesc.Format = DXGI_FORMAT_UNKNOWN;
      uavDesc.Buffer.StructureByteStride = someProperties.myStructureSize;
      uavDesc.Buffer.FirstElement = someProperties.myOffset / someProperties.myStructureSize;
      ASSERT( someProperties.mySize / someProperties.myStructureSize <= UINT_MAX );
      uavDesc.Buffer.NumElements = static_cast< uint >( someProperties.mySize / someProperties.myStructureSize );
    } else {
      ASSERT( someProperties.myFormat != DataFormat::UNKNOWN, "Typed buffer-UAV needs a proper format" );
      const DataFormat       format = someProperties.myFormat;
      const DataFormatInfo & formatInfo = DataFormatInfo::GetFormatInfo( format );
      uavDesc.Format = RenderCore_PlatformDX12::ResolveFormat( format );
      uavDesc.Buffer.FirstElement = someProperties.myOffset / BITS_TO_BYTES( formatInfo.myBitsPerPixel );
      ASSERT( someProperties.mySize / BITS_TO_BYTES( formatInfo.myBitsPerPixel ) <= UINT_MAX );
      uavDesc.Buffer.NumElements =
          static_cast< uint >( someProperties.mySize / BITS_TO_BYTES( formatInfo.myBitsPerPixel ) );
    }

    RenderCore::GetPlatformDX12()->GetDevice()->CreateUnorderedAccessView( dataDx12->myResource.Get(), nullptr,
                                                                           &uavDesc, aDescriptor.myCpuHandle );
    return true;
  }
  //---------------------------------------------------------------------------//
  bool GpuBufferViewDX12::CreateCBVdescriptor( const GpuBuffer *               aBuffer,
                                               const GpuBufferViewProperties & someProperties,
                                               const DescriptorDX12 &          aDescriptor ) {
    const GpuResourceDataDX12 * dataDx12 = aBuffer->GetDX12Data();

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
    cbvDesc.BufferLocation = dataDx12->myResource->GetGPUVirtualAddress() + someProperties.myOffset;

    ASSERT( someProperties.mySize < UINT_MAX );
    cbvDesc.SizeInBytes = ( uint ) someProperties.mySize;

    RenderCore::GetPlatformDX12()->GetDevice()->CreateConstantBufferView( &cbvDesc, aDescriptor.myCpuHandle );
    return true;
  }
  //---------------------------------------------------------------------------//
  void * GpuBufferDX12::Map_Internal( uint64 anOffset, uint64 aSize ) const {
    D3D12_RANGE range;
    range.Begin = anOffset;
    range.End = range.Begin + aSize;

    void * mappedData = nullptr;
    ASSERT_HRESULT( GetDX12Data()->myResource->Map( 0, &range, &mappedData ) );

    return mappedData;
  }
  //---------------------------------------------------------------------------//
  void GpuBufferDX12::Unmap_Internal( GpuResourceMapMode aMapMode, uint64 anOffset /* = 0u */,
                                      uint64 aSize /* = UINT64_MAX */ ) const {
    D3D12_RANGE range;
    range.Begin = anOffset;
    range.End = range.Begin + aSize;

    // Pass an invalid range to Unmap() if the resource hasn't been written to on CPU
    if ( aMapMode == GpuResourceMapMode::READ || aMapMode == GpuResourceMapMode::READ_UNSYNCHRONIZED )
      range.End = 0;

    GetDX12Data()->myResource->Unmap( 0u, &range );
  }
  //---------------------------------------------------------------------------//
}  // namespace Fancy

#endif