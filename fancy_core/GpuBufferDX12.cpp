#include "GpuBufferDX12.h"
#include "Fancy.h"
#include "DescriptorHeapDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"
#include "AdapterDX12.h"
#include "GpuResourceStorageDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  GpuBufferDX12::GpuBufferDX12()
  {
    memset(&myVertexBufferView, 0, sizeof(myVertexBufferView));
    memset(&myIndexBufferView, 0, sizeof(myIndexBufferView));
  }
//---------------------------------------------------------------------------//
  GpuBufferDX12::~GpuBufferDX12()
  {
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::Create(const GpuBufferCreationParams& someParameters, const void* pInitialData)
  {
    Destroy();

    GpuResourceStorageDX12* storageDx12 = new GpuResourceStorageDX12();
    myStorage.reset(storageDx12);

    ASSERT(someParameters.uElementSizeBytes > 0 && someParameters.uNumElements > 0,
      "Invalid buffer size specified");

    GpuBufferCreationParams* pBaseParams = &myParameters;
    *pBaseParams = someParameters;

    const bool wantsShaderResourceView = 
      (someParameters.myUsageFlags & (uint)GpuBufferUsage::SHADER_BUFFER) != 0;

    const bool wantsVboView =
      (someParameters.myUsageFlags & (uint)GpuBufferUsage::VERTEX_BUFFER) != 0;

    const bool wantsIboView =
      (someParameters.myUsageFlags & (uint)GpuBufferUsage::INDEX_BUFFER) != 0;

    const bool wantsConstantBufferView =
      (someParameters.myUsageFlags & (uint)GpuBufferUsage::CONSTANT_BUFFER) != 0;

    myAlignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    if (wantsConstantBufferView)
      myAlignment = 256u;

    const uint64 actualWidthBytesWithAlignment =
      MathUtil::Align(someParameters.uNumElements * someParameters.uElementSizeBytes, myAlignment);

    D3D12_RESOURCE_DESC resourceDesc;
    memset(&resourceDesc, 0, sizeof(resourceDesc));
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = actualWidthBytesWithAlignment;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.Flags = someParameters.myIsShaderWritable ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

    myUsageState = GpuResourceState::RESOURCE_STATE_COMMON;

    const GpuMemoryAccessType gpuMemAccess = (GpuMemoryAccessType)someParameters.myAccessType;
    switch(gpuMemAccess) 
    { 
      case GpuMemoryAccessType::NO_CPU_ACCESS: 
        myUsageState = GpuResourceState::RESOURCE_STATE_GENERIC_READ;
        break;
      case GpuMemoryAccessType::CPU_WRITE: 
        myUsageState = GpuResourceState::RESOURCE_STATE_GENERIC_READ;
        break;
      case GpuMemoryAccessType::CPU_READ: 
        myUsageState = GpuResourceState::RESOURCE_STATE_COPY_DEST;
        break;
    }

    RenderCore_PlatformDX12* dx12Platform = RenderCore::GetPlatformDX12();
    D3D12_RESOURCE_STATES usageStateDX12 = Adapter::toNativeType(myUsageState);
    ID3D12Device* device = dx12Platform->GetDevice();
        
    GpuMemoryAllocationDX12 gpuMemory = dx12Platform->AllocateGpuMemory(GpuMemoryType::BUFFER, gpuMemAccess, actualWidthBytesWithAlignment, myAlignment);
    ASSERT(gpuMemory.myHeap != nullptr);
    
    uint64 alignedHeapOffset = MathUtil::Align(gpuMemory.myOffsetInHeap, myAlignment);
    CheckD3Dcall(device->CreatePlacedResource(gpuMemory.myHeap, alignedHeapOffset, &resourceDesc, usageStateDX12, nullptr, IID_PPV_ARGS(&storageDx12->myResource)));

    storageDx12->myGpuMemory = gpuMemory;

    if (pInitialData != nullptr)
    {
      if (gpuMemAccess == GpuMemoryAccessType::CPU_WRITE)
      {
        void* dest = Lock(GpuResoruceLockOption::WRITE);
        ASSERT(dest != nullptr);
        memcpy(dest, pInitialData, someParameters.uNumElements * someParameters.uElementSizeBytes);
        Unlock();
      }
      else
      {
        RenderCore::UpdateBufferData(this, 0u, pInitialData, someParameters.uNumElements * someParameters.uElementSizeBytes);
      }
    }

    if (someParameters.myCreateDerivedViews)
    {
      if (wantsShaderResourceView)
      {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.FirstElement = 0;
        srvDesc.Buffer.NumElements = myParameters.uNumElements;
        srvDesc.Buffer.StructureByteStride = myParameters.uElementSizeBytes;
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

        mySrvDescriptor = dx12Platform->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        dx12Platform->GetDevice()->CreateShaderResourceView(storageDx12->myResource.Get(), &srvDesc, mySrvDescriptor.myCpuHandle);
      }

      if (someParameters.myIsShaderWritable)
      {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement = 0;
        uavDesc.Buffer.NumElements = myParameters.uNumElements;
        uavDesc.Buffer.StructureByteStride = myParameters.uElementSizeBytes;
        uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

        myUavDescriptor = dx12Platform->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        dx12Platform->GetDevice()->CreateUnorderedAccessView(storageDx12->myResource.Get(), nullptr, &uavDesc, myUavDescriptor.myCpuHandle);
      }

      if (wantsConstantBufferView)
      {
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
        ASSERT(actualWidthBytesWithAlignment <= UINT_MAX);
        cbvDesc.SizeInBytes = static_cast<uint>(actualWidthBytesWithAlignment);
        cbvDesc.BufferLocation = storageDx12->myResource->GetGPUVirtualAddress();

        myCbvDescriptor = dx12Platform->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        dx12Platform->GetDevice()->CreateConstantBufferView(&cbvDesc, myCbvDescriptor.myCpuHandle);
      }

      if (wantsVboView)
      {
        myVertexBufferView.BufferLocation = storageDx12->myResource->GetGPUVirtualAddress();
        myVertexBufferView.SizeInBytes = myParameters.uNumElements * myParameters.uElementSizeBytes;
        myVertexBufferView.StrideInBytes = myParameters.uElementSizeBytes;
      }

      if (wantsIboView)
      {
        if (myParameters.uElementSizeBytes == 2u)
          myIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
        else if (myParameters.uElementSizeBytes == 4u)
          myIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
        else
          ASSERT(false, "Unsupported Index buffer stride");

        myIndexBufferView.BufferLocation = storageDx12->myResource->GetGPUVirtualAddress();
        myIndexBufferView.SizeInBytes = myParameters.uNumElements * myParameters.uElementSizeBytes;
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
      RenderCore::GetPlatformDX12()->FreeGpuMemory(storageDx12->myGpuMemory);
    }

    myStorage = nullptr;
    memset(&myVertexBufferView, 0, sizeof(myVertexBufferView));
    memset(&myIndexBufferView, 0, sizeof(myIndexBufferView));
  }
//---------------------------------------------------------------------------//
  void* GpuBufferDX12::Lock(GpuResoruceLockOption eLockOption, uint uOffsetElements /* = 0u */, uint uNumElements /* = 0u */) const
  {
    ASSERT(uOffsetElements + uNumElements <= myParameters.uNumElements);

    if (uNumElements == 0u)
      uNumElements = myParameters.uNumElements - uOffsetElements;

    D3D12_RANGE range;
    range.Begin = uOffsetElements * myParameters.uElementSizeBytes;
    range.End = range.Begin + uNumElements * myParameters.uElementSizeBytes;

    const bool isCpuWritable = myParameters.myAccessType == (uint)GpuMemoryAccessType::CPU_WRITE;
    const bool isCpuReadable = myParameters.myAccessType == (uint)GpuMemoryAccessType::CPU_READ;

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
  void GpuBufferDX12::Unlock(uint anOffsetElements /* = 0u */, uint aNumElements /* = 0u */) const
  {
    if (anOffsetElements == 0u && aNumElements == 0u)
      aNumElements = myParameters.uNumElements;

    ASSERT(anOffsetElements + aNumElements <= myParameters.uNumElements);

    D3D12_RANGE range;
    range.Begin = anOffsetElements * myParameters.uElementSizeBytes;
    range.End = range.Begin + aNumElements * myParameters.uElementSizeBytes;

    const GpuResourceStorageDX12* storageDx12 = static_cast<GpuResourceStorageDX12*>(myStorage.get());
    storageDx12->myResource->Unmap(0u, &range);
  }
//---------------------------------------------------------------------------//
  const DescriptorDX12* GpuBufferDX12::GetDescriptor(DescriptorType aType, uint anIndex) const
  {
    switch (aType)
    {
    case DescriptorType::DEFAULT_READ: 
    case DescriptorType::DEFAULT_READ_DEPTH: 
    case DescriptorType::DEFAULT_READ_STENCIL: 
      return &mySrvDescriptor;
    case DescriptorType::READ_WRITE:
      return &myUavDescriptor;
    case DescriptorType::CONSTANT_BUFFER:
      return &myCbvDescriptor;
    }

    return nullptr;
  }
//---------------------------------------------------------------------------//
}
