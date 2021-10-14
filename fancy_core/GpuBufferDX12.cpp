#include "fancy_core_precompile.h"
#include "GpuBufferDX12.h"

#include "StringUtil.h"
#include "RenderCore.h"

#include "RenderCore_PlatformDX12.h"
#include "GpuResourceDataDX12.h"
#include "GpuResourceViewDataDX12.h"
#include "CommandList.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
//---------------------------------------------------------------------------//
  GpuBufferDX12::~GpuBufferDX12()
  {
    Destroy();
  }
//---------------------------------------------------------------------------//
  bool GpuBufferDX12::IsValid() const
  {
    return GetData() != nullptr && GetData()->myResource.Get() != nullptr;
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::SetName(const char* aName)
  {
    GpuBuffer::SetName(aName);

    if (GpuResourceDataDX12* dataDx12 = GetData())
    {
      eastl::wstring wName = StringUtil::ToWideString(myName);
      dataDx12->myResource->SetName(wName.c_str());
    }
  }
//---------------------------------------------------------------------------//
  GpuResourceDataDX12* GpuBufferDX12::GetData() const
  {
    return !myNativeData.has_value() ? nullptr : const_cast<GpuResourceDataDX12*>(eastl::any_cast<GpuResourceDataDX12>(&myNativeData));
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::Create(const GpuBufferProperties& someProperties, const char* aName /*= nullptr*/, const void* pInitialData /*= nullptr*/)
  {
    ASSERT(someProperties.myElementSizeBytes > 0 && someProperties.myNumElements > 0, "Invalid buffer size specified");

    Destroy();
    GpuResourceDataDX12 dataDx12;
    
    myProperties = someProperties;
    myName = aName != nullptr ? aName : "GpuBuffer_Unnamed";

    myAlignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    if ((someProperties.myBindFlags & (uint) GpuBufferBindFlags::CONSTANT_BUFFER) != 0u)
      myAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;

    const uint64 pitch = MathUtil::Align(someProperties.myNumElements * someProperties.myElementSizeBytes, myAlignment);

    D3D12_RESOURCE_DESC resourceDesc;
    memset(&resourceDesc, 0, sizeof(resourceDesc));
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
    resourceDesc.Flags = someProperties.myIsShaderWritable ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

    D3D12_RESOURCE_STATES readStateMask = D3D12_RESOURCE_STATE_GENERIC_READ;
    D3D12_RESOURCE_STATES writeStateMask = D3D12_RESOURCE_STATE_COPY_DEST;
    if (someProperties.myIsShaderWritable)
      writeStateMask |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

    if (!(someProperties.myBindFlags & (uint)GpuBufferBindFlags::VERTEX_BUFFER) &&
        !(someProperties.myBindFlags & (uint)GpuBufferBindFlags::CONSTANT_BUFFER))
      readStateMask = readStateMask & ~D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    if (!(someProperties.myBindFlags & (uint)GpuBufferBindFlags::INDEX_BUFFER))
      readStateMask = readStateMask & ~D3D12_RESOURCE_STATE_INDEX_BUFFER;
    if (!(someProperties.myBindFlags & (uint)GpuBufferBindFlags::SHADER_BUFFER))
    {
      readStateMask = readStateMask & ~D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

      if ((someProperties.myBindFlags & (uint) GpuBufferBindFlags::RAYTRACING_SHADER_BINDING_TABLE) == 0)
        readStateMask = readStateMask & ~D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    }

    // In most cases, UAV resources will directly be used as such so start with that as an initial state
    D3D12_RESOURCE_STATES initialStates = someProperties.myIsShaderWritable ? D3D12_RESOURCE_STATE_UNORDERED_ACCESS : (D3D12_RESOURCE_STATE_GENERIC_READ & readStateMask) & writeStateMask;
    bool canChangeStates = true;
    if (someProperties.myCpuAccess == CpuMemoryAccessType::CPU_WRITE)  // Upload heap
    {
      canChangeStates = false;
      initialStates = D3D12_RESOURCE_STATE_GENERIC_READ;
    }
    else if (someProperties.myCpuAccess == CpuMemoryAccessType::CPU_READ)  // Readback heap
    {
      canChangeStates = false;
      initialStates = D3D12_RESOURCE_STATE_COPY_DEST;
    }

    if (someProperties.myBindFlags & (uint)GpuBufferBindFlags::RT_ACCELERATION_STRUCTURE_STORAGE)
    {
      initialStates = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
      writeStateMask |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
    }

    GpuSubresourceHazardDataDX12 subHazardData;
    subHazardData.myContext = CommandListType::Graphics;
    subHazardData.myStates = initialStates;

    GpuResourceHazardDataDX12* hazardData = &dataDx12.myHazardData;
    *hazardData = GpuResourceHazardDataDX12();
    hazardData->myCanChangeStates = canChangeStates;
    hazardData->mySubresources.push_back(subHazardData);
    hazardData->myReadStates = readStateMask;
    hazardData->myWriteStates = writeStateMask;

    mySubresources = SubresourceRange(0u, 1u, 0u, 1u, 0u, 1u);

    RenderCore_PlatformDX12* dx12Platform = RenderCore::GetPlatformDX12();
    ID3D12Device* device = dx12Platform->GetDevice();

    GpuMemoryAllocationDX12 gpuMemory = dx12Platform->AllocateGpuMemory(GpuMemoryType::BUFFER, someProperties.myCpuAccess, pitch, myAlignment, myName.c_str());
    ASSERT(gpuMemory.myHeap != nullptr);

    const uint64 alignedHeapOffset = MathUtil::Align(gpuMemory.myOffsetInHeap, myAlignment);
    ASSERT_HRESULT(device->CreatePlacedResource(gpuMemory.myHeap, alignedHeapOffset, &resourceDesc, initialStates, nullptr, IID_PPV_ARGS(&dataDx12.myResource)));

    eastl::wstring wName = StringUtil::ToWideString(myName);
    dataDx12.myResource->SetName(wName.c_str());
    dataDx12.myGpuMemory = gpuMemory;

    myNativeData = dataDx12;

    if (pInitialData != nullptr)
    {
      if (someProperties.myCpuAccess == CpuMemoryAccessType::CPU_WRITE)
      {
        void* dest = Map(GpuResourceMapMode::WRITE_UNSYNCHRONIZED);
        ASSERT(dest != nullptr);
        memcpy(dest, pInitialData, someProperties.myNumElements * someProperties.myElementSizeBytes);
        Unmap(GpuResourceMapMode::WRITE_UNSYNCHRONIZED);
      }
      else
      {
        CommandList* ctx = RenderCore::BeginCommandList(CommandListType::Graphics);
        ctx->UpdateBufferData(this, 0u, pInitialData, someProperties.myNumElements * someProperties.myElementSizeBytes);
        RenderCore::ExecuteAndFreeCommandList(ctx, SyncMode::BLOCKING);
      }
    }
  }
//---------------------------------------------------------------------------//
  uint64 GpuBufferDX12::GetDeviceAddress() const
  {
    if (GetData() == nullptr)
      return 0;

    return GetData()->myResource->GetGPUVirtualAddress();
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::Destroy()
  {
    GpuResourceDataDX12* dataDx12 = GetData();
    if (dataDx12 != nullptr)
    {
      dataDx12->myResource.Reset();

      if(dataDx12->myGpuMemory.myHeap != nullptr)
        RenderCore::GetPlatformDX12()->ReleaseGpuMemory(dataDx12->myGpuMemory);
    }

    myNativeData.reset();
    myProperties = GpuBufferProperties();
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  GpuBufferViewDX12::GpuBufferViewDX12(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties)
    : GpuBufferView::GpuBufferView(aBuffer, someProperties)
  {
    GpuResourceViewDataDX12 nativeData;

    bool success = false;
    if (someProperties.myIsConstantBuffer)
    {
      ASSERT(myType == GpuResourceViewType::CBV);
      nativeData.myDescriptor = RenderCore::GetPlatformDX12()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, "GpuBufferView");
      ASSERT(nativeData.myDescriptor.myCpuHandle.ptr != UINT_MAX);
      success = CreateCBVdescriptor(aBuffer.get(), someProperties, nativeData.myDescriptor);
    }
    else
    {
      GpuBufferViewProperties rawProperties = someProperties;
      rawProperties.myIsRaw = true;
      rawProperties.myIsStructured = false;

      if (someProperties.myIsShaderWritable)
      {
        ASSERT(myType == GpuResourceViewType::UAV);
        nativeData.myDescriptor = RenderCore::GetPlatformDX12()->AllocateShaderVisibleDescriptorForGlobalResource(GLOBAL_RESOURCE_RWBUFFER, "GpuBufferView");
        success = CreateUAVdescriptor(aBuffer.get(), rawProperties, nativeData.myDescriptor);
        myGlobalDescriptorIndex = nativeData.myDescriptor.myGlobalResourceIndex;
      }
      else
      {
        ASSERT(myType == GpuResourceViewType::SRV);
        nativeData.myDescriptor = RenderCore::GetPlatformDX12()->AllocateShaderVisibleDescriptorForGlobalResource(GLOBAL_RESOURCE_BUFFER, "GpuBufferView");
        success = CreateSRVdescriptor(aBuffer.get(), rawProperties, nativeData.myDescriptor);
        myGlobalDescriptorIndex = nativeData.myDescriptor.myGlobalResourceIndex;
      }
    }

    ASSERT(success);

    myNativeData = nativeData;
    mySubresourceRange = SubresourceRange(0u, 1u, 0u, 1u, 0u, 1u);
    myCoversAllSubresources = true;
  }
//---------------------------------------------------------------------------//
  GpuBufferViewDX12::~GpuBufferViewDX12()
  {
    const GpuResourceViewDataDX12& viewData = eastl::any_cast<const GpuResourceViewDataDX12&>(myNativeData);
    RenderCore::GetPlatformDX12()->FreeDescriptor(viewData.myDescriptor);
  }
//---------------------------------------------------------------------------//
  bool GpuBufferViewDX12::CreateSRVdescriptor(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someProperties, const DescriptorDX12& aDescriptor)
  {
    GpuResourceDataDX12* dataDx12 = static_cast<const GpuBufferDX12*>(aBuffer)->GetData();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    memset(&srvDesc, 0u, sizeof(srvDesc));

    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    if (someProperties.myIsRaw)
    {
      srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
      srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
      srvDesc.Buffer.FirstElement = someProperties.myOffset / 4;
      ASSERT(someProperties.mySize / 4 <= UINT_MAX);
      srvDesc.Buffer.NumElements = static_cast<uint>(someProperties.mySize / 4);
    }
    else if (someProperties.myIsStructured)
    {
      srvDesc.Format = DXGI_FORMAT_UNKNOWN;
      srvDesc.Buffer.StructureByteStride = someProperties.myStructureSize;
      srvDesc.Buffer.FirstElement = someProperties.myOffset / someProperties.myStructureSize;
      ASSERT(someProperties.mySize / someProperties.myStructureSize <= UINT_MAX);
      srvDesc.Buffer.NumElements = static_cast<uint>(someProperties.mySize / someProperties.myStructureSize);
    }
    else
    {
      ASSERT(someProperties.myFormat != DataFormat::UNKNOWN, "Typed buffer-SRV needs a proper format");
      const DataFormat format = someProperties.myFormat;
      const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(format);
      srvDesc.Format = RenderCore_PlatformDX12::ResolveFormat(format);
      srvDesc.Buffer.FirstElement = someProperties.myOffset / formatInfo.mySizeBytes;
      ASSERT(someProperties.mySize / formatInfo.mySizeBytes <= UINT_MAX);
      srvDesc.Buffer.NumElements = static_cast<uint>(someProperties.mySize / formatInfo.mySizeBytes);
    }

    RenderCore::GetPlatformDX12()->GetDevice()->CreateShaderResourceView(dataDx12->myResource.Get(), &srvDesc, aDescriptor.myCpuHandle);
    return true;
  }
//---------------------------------------------------------------------------//
  bool GpuBufferViewDX12::CreateUAVdescriptor(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someProperties, const DescriptorDX12& aDescriptor)
  {
    GpuResourceDataDX12* dataDx12 = static_cast<const GpuBufferDX12*>(aBuffer)->GetData();

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    memset(&uavDesc, 0u, sizeof(uavDesc));

    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

    if (someProperties.myIsRaw)
    {
      uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
      uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
      uavDesc.Buffer.FirstElement = someProperties.myOffset / 4;
      ASSERT(someProperties.mySize / 4 <= UINT_MAX);
      uavDesc.Buffer.NumElements = static_cast<uint>(someProperties.mySize / 4);
    }
    else if (someProperties.myIsStructured)
    {
      uavDesc.Format = DXGI_FORMAT_UNKNOWN;
      uavDesc.Buffer.StructureByteStride = someProperties.myStructureSize;
      uavDesc.Buffer.FirstElement = someProperties.myOffset / someProperties.myStructureSize;
      ASSERT(someProperties.mySize / someProperties.myStructureSize <= UINT_MAX);
      uavDesc.Buffer.NumElements = static_cast<uint>(someProperties.mySize / someProperties.myStructureSize);
    }
    else
    {
      ASSERT(someProperties.myFormat != DataFormat::UNKNOWN, "Typed buffer-UAV needs a proper format");
      const DataFormat format = someProperties.myFormat;
      const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(format);
      uavDesc.Format = RenderCore_PlatformDX12::ResolveFormat(format);
      uavDesc.Buffer.FirstElement = someProperties.myOffset / formatInfo.mySizeBytes;
      ASSERT(someProperties.mySize / formatInfo.mySizeBytes <= UINT_MAX);
      uavDesc.Buffer.NumElements = static_cast<uint>(someProperties.mySize / formatInfo.mySizeBytes);
    }

    RenderCore::GetPlatformDX12()->GetDevice()->CreateUnorderedAccessView(dataDx12->myResource.Get(), nullptr, &uavDesc, aDescriptor.myCpuHandle);
    return true;
  }
//---------------------------------------------------------------------------//
  bool GpuBufferViewDX12::CreateCBVdescriptor(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someProperties, const DescriptorDX12& aDescriptor)
  {
    GpuResourceDataDX12* dataDx12 = static_cast<const GpuBufferDX12*>(aBuffer)->GetData();

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
    cbvDesc.BufferLocation = dataDx12->myResource->GetGPUVirtualAddress() + someProperties.myOffset;

    ASSERT(someProperties.mySize < UINT_MAX);
    cbvDesc.SizeInBytes = (uint) someProperties.mySize;
    
    RenderCore::GetPlatformDX12()->GetDevice()->CreateConstantBufferView(&cbvDesc, aDescriptor.myCpuHandle);
    return true;
  }
//---------------------------------------------------------------------------//
  void* GpuBufferDX12::Map_Internal(uint64 anOffset, uint64 aSize) const
  {
    D3D12_RANGE range;
    range.Begin = anOffset;
    range.End = range.Begin + aSize;

    void* mappedData = nullptr;
    ASSERT_HRESULT(GetData()->myResource->Map(0, &range, &mappedData));

    return mappedData;
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::Unmap_Internal(GpuResourceMapMode aMapMode, uint64 anOffset /* = 0u */, uint64 aSize /* = UINT64_MAX */) const
  {
    D3D12_RANGE range;
    range.Begin = anOffset;
    range.End = range.Begin + aSize;

    // Pass an invalid range to Unmap() if the resource hasn't been written to on CPU
    if (aMapMode == GpuResourceMapMode::READ || aMapMode == GpuResourceMapMode::READ_UNSYNCHRONIZED)
      range.End = 0;

    GetData()->myResource->Unmap(0u, &range);
  }
//---------------------------------------------------------------------------//
}

#endif