#include "fancy_core_precompile.h"
#include "GpuBufferDX12.h"

#include "StringUtil.h"
#include "RenderCore.h"

#include "RenderCore_PlatformDX12.h"
#include "GpuResourceDataDX12.h"
#include "GpuResourceViewDX12.h"

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
    if (someProperties.myUsage == GpuBufferUsage::CONSTANT_BUFFER)
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

    const CpuMemoryAccessType cpuMemAccess = (CpuMemoryAccessType)someProperties.myCpuAccess;

    D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;
    D3D12_RESOURCE_STATES readState = D3D12_RESOURCE_STATE_GENERIC_READ;
    D3D12_RESOURCE_STATES writeState = D3D12_RESOURCE_STATE_COPY_DEST;
    if (someProperties.myIsShaderWritable)
      writeState |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

    bool canChangeStates = true;
    if (cpuMemAccess == CpuMemoryAccessType::CPU_WRITE)  // Upload heap
    {
      initialState = D3D12_RESOURCE_STATE_GENERIC_READ;
      canChangeStates = false;
    }
    else if (cpuMemAccess == CpuMemoryAccessType::CPU_READ)  // Readback heap
    {
      initialState = D3D12_RESOURCE_STATE_COPY_DEST;
      canChangeStates = false;
    }
    else
    {
      // TODO: Rework the usage-mode and allow for multiple usages ("bindFlags") at the same time. It should be possible to make a vertexBuffer that is also a shaderBuffer
      switch (someProperties.myUsage)
      {
      case GpuBufferUsage::STAGING_UPLOAD:
        initialState = D3D12_RESOURCE_STATE_GENERIC_READ; // Required for upload-heaps according to the D3D12-docs
        break;
      case GpuBufferUsage::STAGING_READBACK:
        initialState = D3D12_RESOURCE_STATE_COPY_DEST;
        break;
      case GpuBufferUsage::VERTEX_BUFFER:
      case GpuBufferUsage::CONSTANT_BUFFER:
        readState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_COPY_SOURCE;
        initialState = readState;
        break;
      case GpuBufferUsage::INDEX_BUFFER:
        readState = D3D12_RESOURCE_STATE_INDEX_BUFFER | D3D12_RESOURCE_STATE_COPY_SOURCE;
        initialState = readState;
        break;
      case GpuBufferUsage::SHADER_BUFFER:
        readState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_COPY_SOURCE;
        initialState = readState;
        if (someProperties.myIsShaderWritable)
          initialState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        break;
      default: ASSERT(false, "Missing implementation");
      }
    }

    myHazardData = GpuResourceHazardData();
    myHazardData.myDx12Data.mySubresourceStates.push_back(initialState);
    myHazardData.myDx12Data.myReadStates = readState;
    myHazardData.myDx12Data.myWriteStates = writeState;
    myHazardData.mySubresourceContexts.push_back(CommandListType::Graphics);
    myHazardData.myCanChangeStates = canChangeStates;

    myNumSubresources = 1u;
    myNumSubresourcesPerPlane = 1u;
    myNumPlanes = 1u;

    RenderCore_PlatformDX12* dx12Platform = RenderCore::GetPlatformDX12();
    ID3D12Device* device = dx12Platform->GetDevice();

    GpuMemoryAllocationDX12 gpuMemory = dx12Platform->AllocateGpuMemory(GpuMemoryType::BUFFER, cpuMemAccess, pitch, myAlignment, myName.c_str());
    ASSERT(gpuMemory.myHeap != nullptr);

    const uint64 alignedHeapOffset = MathUtil::Align(gpuMemory.myOffsetInHeap, myAlignment);
    CheckD3Dcall(device->CreatePlacedResource(gpuMemory.myHeap, alignedHeapOffset, &resourceDesc, initialState, nullptr, IID_PPV_ARGS(&dataDx12->myResource)));

    std::wstring wName = StringUtil::ToWideString(myName);
    dataDx12->myResource->SetName(wName.c_str());

    dataDx12->myGpuMemory = gpuMemory;

    if (pInitialData != nullptr)
    {
      if (cpuMemAccess == CpuMemoryAccessType::CPU_WRITE)
      {
        void* dest = Map(GpuResourceMapMode::WRITE_UNSYNCHRONIZED);
        ASSERT(dest != nullptr);
        memcpy(dest, pInitialData, someProperties.myNumElements * someProperties.myElementSizeBytes);
        Unmap(GpuResourceMapMode::WRITE_UNSYNCHRONIZED);
      }
      else
      {
        RenderCore::UpdateBufferData(this, 0u, pInitialData, someProperties.myNumElements * someProperties.myElementSizeBytes, SyncMode::BLOCKING);
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
    ASSERT(nativeData.myDescriptor.myCpuHandle.ptr != 0u);

    bool success = false;
    if (someProperties.myIsConstantBuffer)
    {
      nativeData.myType = GpuResourceViewDataDX12::CBV;
      success = CreateCBV(aBuffer.get(), someProperties, nativeData.myDescriptor);
    }
    else if (someProperties.myIsShaderWritable)
    {
      nativeData.myType = GpuResourceViewDataDX12::UAV;
      success = CreateUAV(aBuffer.get(), someProperties, nativeData.myDescriptor);
    }
    else
    {
      nativeData.myType = GpuResourceViewDataDX12::SRV;
      success = CreateSRV(aBuffer.get(), someProperties, nativeData.myDescriptor);
    }

    ASSERT(success);

    myNativeData = nativeData;
    mySubresources->push_back(0u);
    myCoversAllSubresources = true;
  }
//---------------------------------------------------------------------------//
  GpuBufferViewDX12::~GpuBufferViewDX12()
  {
    const GpuResourceViewDataDX12& viewData = myNativeData.To<GpuResourceViewDataDX12>();
    RenderCore::GetPlatformDX12()->ReleaseDescriptor(viewData.myDescriptor);
  }
//---------------------------------------------------------------------------//
  bool GpuBufferViewDX12::CreateSRV(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someProperties, const DescriptorDX12& aDescriptor)
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
      srvDesc.Format = RenderCore_PlatformDX12::GetDXGIformat(format);
      srvDesc.Buffer.FirstElement = someProperties.myOffset / formatInfo.mySizeBytes;
      ASSERT(someProperties.mySize / formatInfo.mySizeBytes <= UINT_MAX);
      srvDesc.Buffer.NumElements = static_cast<uint>(someProperties.mySize / formatInfo.mySizeBytes);
    }

    RenderCore::GetPlatformDX12()->GetDevice()->CreateShaderResourceView(dataDx12->myResource.Get(), &srvDesc, aDescriptor.myCpuHandle);
    return true;
  }
//---------------------------------------------------------------------------//
  bool GpuBufferViewDX12::CreateUAV(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someProperties, const DescriptorDX12& aDescriptor)
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
      uavDesc.Format = RenderCore_PlatformDX12::GetDXGIformat(format);
      uavDesc.Buffer.FirstElement = someProperties.myOffset / formatInfo.mySizeBytes;
      ASSERT(someProperties.mySize / formatInfo.mySizeBytes <= UINT_MAX);
      uavDesc.Buffer.NumElements = static_cast<uint>(someProperties.mySize / formatInfo.mySizeBytes);
    }

    RenderCore::GetPlatformDX12()->GetDevice()->CreateUnorderedAccessView(dataDx12->myResource.Get(), nullptr, &uavDesc, aDescriptor.myCpuHandle);
    return true;
  }
//---------------------------------------------------------------------------//
  bool GpuBufferViewDX12::CreateCBV(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someProperties, const DescriptorDX12& aDescriptor)
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
  void* GpuBufferDX12::Map(GpuResourceMapMode aMapMode, uint64 anOffset, uint64 aSize) const
  {
    const uint64 bufferSize = myProperties.myNumElements * myProperties.myElementSizeBytes;
    aSize = glm::min(bufferSize, aSize);
    ASSERT(anOffset + aSize <= bufferSize);

    const bool isCpuWritable = myProperties.myCpuAccess == CpuMemoryAccessType::CPU_WRITE;
    const bool isCpuReadable = myProperties.myCpuAccess == CpuMemoryAccessType::CPU_READ;

    const bool wantsWrite = aMapMode == GpuResourceMapMode::WRITE_UNSYNCHRONIZED || aMapMode == GpuResourceMapMode::WRITE;
    const bool wantsRead = aMapMode == GpuResourceMapMode::READ_UNSYNCHRONIZED || aMapMode == GpuResourceMapMode::READ;

    if ((wantsWrite && !isCpuWritable) || (wantsRead && !isCpuReadable))
      return nullptr;

    const bool needsWait = aMapMode == GpuResourceMapMode::READ || aMapMode == GpuResourceMapMode::WRITE;
    if (needsWait)
      RenderCore::WaitForResourceIdle(this);

    D3D12_RANGE range;
    range.Begin = anOffset;
    range.End = range.Begin + aSize;

    void* mappedData = nullptr;
    CheckD3Dcall(GetData()->myResource->Map(0, &range, &mappedData));

    return mappedData;
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::Unmap(GpuResourceMapMode aMapMode, uint64 anOffset /* = 0u */, uint64 aSize /* = UINT64_MAX */) const
  {
    const uint64 bufferSize = myProperties.myNumElements * myProperties.myElementSizeBytes;
    aSize = glm::min(bufferSize, aSize);
    ASSERT(anOffset + aSize <= bufferSize);

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
