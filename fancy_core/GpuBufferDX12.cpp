#include "GpuBufferDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"
#include "AdapterDX12.h"
#include "GpuResourceStorageDX12.h"
#include "GpuResourceViewDX12.h"
#include "StringUtil.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  GpuBufferDX12::GpuBufferDX12()
  {
  }
//---------------------------------------------------------------------------//
  GpuBufferDX12::~GpuBufferDX12()
  {
    Destroy();
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::Create(const GpuBufferProperties& someProperties, const char* aName /*= nullptr*/, const void* pInitialData /*= nullptr*/)
  {
    ASSERT(someProperties.myElementSizeBytes > 0 && someProperties.myNumElements > 0, "Invalid buffer size specified");

    Destroy();

    GpuResourceStorageDX12* storageDx12 = new GpuResourceStorageDX12();
    myStorage.reset(storageDx12);

    myProperties = someProperties;
    myName = aName;

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

    const GpuMemoryAccessType gpuMemAccess = (GpuMemoryAccessType)someProperties.myCpuAccess;

    D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;
    D3D12_RESOURCE_STATES readState = D3D12_RESOURCE_STATE_GENERIC_READ;
    bool canChangeStates = true;

    if (gpuMemAccess == GpuMemoryAccessType::CPU_WRITE)  // Upload heap
    {
      initialState = D3D12_RESOURCE_STATE_GENERIC_READ;
      readState = D3D12_RESOURCE_STATE_GENERIC_READ;
      canChangeStates = false;
    }
    else if (gpuMemAccess == GpuMemoryAccessType::CPU_READ)  // Readback heap
    {
      initialState = D3D12_RESOURCE_STATE_COPY_DEST;
      canChangeStates = false;
    }
    else
    {
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
        initialState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        readState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        break;
      case GpuBufferUsage::INDEX_BUFFER:
        initialState = D3D12_RESOURCE_STATE_INDEX_BUFFER;
        readState = D3D12_RESOURCE_STATE_INDEX_BUFFER;
        break;
      case GpuBufferUsage::SHADER_BUFFER:
        initialState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        readState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        if (someProperties.myIsShaderWritable)
          initialState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        break;
      default: ASSERT(false, "Missing implementation");
      }
    }
    storageDx12->mySubresourceStates.resize(1u);
    storageDx12->mySubresourceStates[0] = initialState;
    storageDx12->mySubresourceContexts.resize(1u);
    storageDx12->mySubresourceContexts[0] = CommandListType::Graphics;
    storageDx12->myReadState = readState;
    storageDx12->myCanChangeStates = canChangeStates;

    RenderCore_PlatformDX12* dx12Platform = RenderCore::GetPlatformDX12();
    ID3D12Device* device = dx12Platform->GetDevice();

    GpuMemoryAllocationDX12 gpuMemory = dx12Platform->AllocateGpuMemory(GpuMemoryType::BUFFER, gpuMemAccess, pitch, myAlignment);
    ASSERT(gpuMemory.myHeap != nullptr);

    const uint64 alignedHeapOffset = MathUtil::Align(gpuMemory.myOffsetInHeap, myAlignment);
    CheckD3Dcall(device->CreatePlacedResource(gpuMemory.myHeap, alignedHeapOffset, &resourceDesc, initialState, nullptr, IID_PPV_ARGS(&storageDx12->myResource)));

    std::wstring wName = StringUtil::ToWideString(myName);
    storageDx12->myResource->SetName(wName.c_str());

    storageDx12->myGpuMemory = gpuMemory;

    if (pInitialData != nullptr)
    {
      if (gpuMemAccess == GpuMemoryAccessType::CPU_WRITE)
      {
        void* dest = Lock(GpuResoruceLockOption::WRITE);
        ASSERT(dest != nullptr);
        memcpy(dest, pInitialData, someProperties.myNumElements * someProperties.myElementSizeBytes);
        Unlock();
      }
      else
      {
        RenderCore::UpdateBufferData(this, 0u, pInitialData, someProperties.myNumElements * someProperties.myElementSizeBytes);
      }
    }
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::Destroy()
  {
    GpuResourceStorageDX12* storageDx12 = (GpuResourceStorageDX12*)myStorage.get();
    if (storageDx12 != nullptr && storageDx12->myGpuMemory.myHeap != nullptr)
    {
      storageDx12->myResource.Reset();
      RenderCore::GetPlatformDX12()->ReleaseGpuMemory(storageDx12->myGpuMemory);
    }

    myStorage = nullptr;
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
    GpuResourceStorageDX12* storageDx12 = (GpuResourceStorageDX12*)aBuffer->myStorage.get();

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
      DataFormat format = RenderCore::GetPlatformDX12()->ResolveFormat(someProperties.myFormat);
      const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(format);
      srvDesc.Format = RenderCore_PlatformDX12::GetDXGIformat(format);
      srvDesc.Buffer.FirstElement = someProperties.myOffset / formatInfo.mySizeBytes;
      ASSERT(someProperties.mySize / formatInfo.mySizeBytes <= UINT_MAX);
      srvDesc.Buffer.NumElements = static_cast<uint>(someProperties.mySize / formatInfo.mySizeBytes);
    }

    RenderCore::GetPlatformDX12()->GetDevice()->CreateShaderResourceView(storageDx12->myResource.Get(), &srvDesc, aDescriptor.myCpuHandle);
    return true;
  }
//---------------------------------------------------------------------------//
  bool GpuBufferViewDX12::CreateUAV(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someProperties, const DescriptorDX12& aDescriptor)
  {
    GpuResourceStorageDX12* storageDx12 = (GpuResourceStorageDX12*)aBuffer->myStorage.get();

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
      DataFormat format = RenderCore::GetPlatformDX12()->ResolveFormat(someProperties.myFormat);
      const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(format);
      uavDesc.Format = RenderCore_PlatformDX12::GetDXGIformat(format);
      uavDesc.Buffer.FirstElement = someProperties.myOffset / formatInfo.mySizeBytes;
      ASSERT(someProperties.mySize / formatInfo.mySizeBytes <= UINT_MAX);
      uavDesc.Buffer.NumElements = static_cast<uint>(someProperties.mySize / formatInfo.mySizeBytes);
    }

    RenderCore::GetPlatformDX12()->GetDevice()->CreateUnorderedAccessView(storageDx12->myResource.Get(), nullptr, &uavDesc, aDescriptor.myCpuHandle);
    return true;
  }
//---------------------------------------------------------------------------//
  bool GpuBufferViewDX12::CreateCBV(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someProperties, const DescriptorDX12& aDescriptor)
  {
    GpuResourceStorageDX12* storageDx12 = (GpuResourceStorageDX12*)aBuffer->myStorage.get();

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
    cbvDesc.BufferLocation = storageDx12->myResource->GetGPUVirtualAddress() + someProperties.myOffset;

    ASSERT(someProperties.mySize < UINT_MAX);
    cbvDesc.SizeInBytes = (uint) someProperties.mySize;
    
    RenderCore::GetPlatformDX12()->GetDevice()->CreateConstantBufferView(&cbvDesc, aDescriptor.myCpuHandle);
    return true;
  }
//---------------------------------------------------------------------------//
  void* GpuBufferDX12::Lock(GpuResoruceLockOption eLockOption, uint64 uOffsetElements /* = 0u */, uint64 uNumElements /* = 0u */) const
  {
    ASSERT(uOffsetElements + uNumElements <= myProperties.myNumElements);

    if (uNumElements == 0u)
      uNumElements = myProperties.myNumElements - uOffsetElements;

    D3D12_RANGE range;
    range.Begin = uOffsetElements * myProperties.myElementSizeBytes;
    range.End = range.Begin + uNumElements * myProperties.myElementSizeBytes;

    const bool isCpuWritable = myProperties.myCpuAccess == GpuMemoryAccessType::CPU_WRITE;
    const bool isCpuReadable = myProperties.myCpuAccess == GpuMemoryAccessType::CPU_READ;

    const bool wantsWrite = eLockOption == GpuResoruceLockOption::READ_WRITE || eLockOption == GpuResoruceLockOption::WRITE;
    const bool wantsRead = eLockOption == GpuResoruceLockOption::READ_WRITE || eLockOption == GpuResoruceLockOption::READ;

    if ((wantsWrite && !isCpuWritable) || (wantsRead && !isCpuReadable))
      return nullptr;

    // TODO: Do something with the current usage type? Transition it into something correct? Early-out?

    const GpuResourceStorageDX12* storageDx12 = static_cast<GpuResourceStorageDX12*>(myStorage.get());

    void* mappedData = nullptr;
    CheckD3Dcall(storageDx12->myResource->Map(0, &range, &mappedData));

    return mappedData;
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::Unlock(uint64 anOffsetElements /* = 0u */, uint64 aNumElements /* = 0u */) const
  {
    if (anOffsetElements == 0u && aNumElements == 0u)
      aNumElements = myProperties.myNumElements;

    ASSERT(anOffsetElements + aNumElements <= myProperties.myNumElements);

    D3D12_RANGE range;
    range.Begin = anOffsetElements * myProperties.myElementSizeBytes;
    range.End = range.Begin + aNumElements * myProperties.myElementSizeBytes;

    const GpuResourceStorageDX12* storageDx12 = static_cast<GpuResourceStorageDX12*>(myStorage.get());
    storageDx12->myResource->Unmap(0u, &range);
  }
//---------------------------------------------------------------------------//
}
