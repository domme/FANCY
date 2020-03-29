#include "fancy_core_precompile.h"
#include "GpuBufferDX12.h"

#include "StringUtil.h"
#include "RenderCore.h"

#include "RenderCore_PlatformDX12.h"
#include "GpuResourceDataDX12.h"
#include "GpuResourceViewDataDX12.h"
#include "CommandList.h"

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
      std::wstring wName = StringUtil::ToWideString(myName);
      dataDx12->myResource->SetName(wName.c_str());
    }
  }
//---------------------------------------------------------------------------//
  GpuResourceDataDX12* GpuBufferDX12::GetData() const
  {
    return myNativeData.IsEmpty() ? nullptr : myNativeData.To<GpuResourceDataDX12*>();
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::Create(const GpuBufferProperties& someProperties, const char* aName /*= nullptr*/, const void* pInitialData /*= nullptr*/)
  {
    ASSERT(someProperties.myElementSizeBytes > 0 && someProperties.myNumElements > 0, "Invalid buffer size specified");

    Destroy();
    GpuResourceDataDX12* dataDx12 = new GpuResourceDataDX12();
    myNativeData = dataDx12;

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
      readStateMask = readStateMask & ~(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    D3D12_RESOURCE_STATES initialStates = (D3D12_RESOURCE_STATE_GENERIC_READ & readStateMask) & writeStateMask;
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


    GpuSubresourceHazardDataDX12 subHazardData;
    subHazardData.myContext = CommandListType::Graphics;
    subHazardData.myStates = initialStates;

    myStateTracking = GpuResourceHazardData();
    myStateTracking.myCanChangeStates = canChangeStates;
    myStateTracking.myDx12Data.mySubresources.push_back(subHazardData);
    myStateTracking.myDx12Data.myReadStates = readStateMask;
    myStateTracking.myDx12Data.myWriteStates = writeStateMask;

    mySubresources = SubresourceRange(0u, 1u, 0u, 1u, 0u, 1u);

    RenderCore_PlatformDX12* dx12Platform = RenderCore::GetPlatformDX12();
    ID3D12Device* device = dx12Platform->GetDevice();

    GpuMemoryAllocationDX12 gpuMemory = dx12Platform->AllocateGpuMemory(GpuMemoryType::BUFFER, someProperties.myCpuAccess, pitch, myAlignment, myName.c_str());
    ASSERT(gpuMemory.myHeap != nullptr);

    const uint64 alignedHeapOffset = MathUtil::Align(gpuMemory.myOffsetInHeap, myAlignment);
    CheckD3Dcall(device->CreatePlacedResource(gpuMemory.myHeap, alignedHeapOffset, &resourceDesc, initialStates, nullptr, IID_PPV_ARGS(&dataDx12->myResource)));

    std::wstring wName = StringUtil::ToWideString(myName);
    dataDx12->myResource->SetName(wName.c_str());

    dataDx12->myGpuMemory = gpuMemory;

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
  void GpuBufferDX12::Destroy()
  {
    GpuResourceDataDX12* dataDx12 = GetData();
    if (dataDx12 != nullptr)
    {
      dataDx12->myResource.Reset();

      if(dataDx12->myGpuMemory.myHeap != nullptr)
        RenderCore::GetPlatformDX12()->ReleaseGpuMemory(dataDx12->myGpuMemory);

      delete dataDx12;
    }

    myNativeData.Clear();
    myProperties = GpuBufferProperties();
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  GpuBufferViewDX12::GpuBufferViewDX12(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties)
    : GpuBufferView::GpuBufferView(aBuffer, someProperties)
  {
    GpuResourceViewDataDX12 nativeData;
    nativeData.myDescriptor = RenderCore::GetPlatformDX12()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, "GpuBufferView");
    ASSERT(nativeData.myDescriptor.myCpuHandle.ptr != UINT_MAX);

    bool success = false;
    if (someProperties.myIsConstantBuffer)
    {
      myType = GpuResourceViewType::CBV;
      success = CreateCBVdescriptor(aBuffer.get(), someProperties, nativeData.myDescriptor);
    }
    else if (someProperties.myIsShaderWritable)
    {
      myType = GpuResourceViewType::UAV;
      success = CreateUAVdescriptor(aBuffer.get(), someProperties, nativeData.myDescriptor);
    }
    else
    {
      myType = GpuResourceViewType::SRV;
      success = CreateSRVdescriptor(aBuffer.get(), someProperties, nativeData.myDescriptor);
    }

    ASSERT(success);

    myNativeData = nativeData;
    mySubresourceRange = SubresourceRange(0u, 1u, 0u, 1u, 0u, 1u);
    myCoversAllSubresources = true;
  }
//---------------------------------------------------------------------------//
  GpuBufferViewDX12::~GpuBufferViewDX12()
  {
    const GpuResourceViewDataDX12& viewData = myNativeData.To<GpuResourceViewDataDX12>();
    RenderCore::GetPlatformDX12()->ReleaseDescriptor(viewData.myDescriptor);
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
    CheckD3Dcall(GetData()->myResource->Map(0, &range, &mappedData));

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
