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
    
    const bool wantsUnorderedAccess = 
      (someParameters.myUsageFlags & (uint)GpuBufferUsage::RESOURCE_BUFFER_RW) 
      || (someParameters.myUsageFlags & (uint)GpuBufferUsage::RESOURCE_BUFFER_LARGE_RW);

    const bool wantsShaderResourceView =
      wantsUnorderedAccess 
      || (someParameters.myUsageFlags & (uint)GpuBufferUsage::RESOURCE_BUFFER)
      || (someParameters.myUsageFlags & (uint)GpuBufferUsage::RESOURCE_BUFFER_LARGE);

    const bool wantsVboView =
      (someParameters.myUsageFlags & (uint)GpuBufferUsage::VERTEX_BUFFER) != 0;

    const bool wantsIboView =
      (someParameters.myUsageFlags & (uint)GpuBufferUsage::INDEX_BUFFER) != 0;

    const bool wantsConstantBufferView =
      (someParameters.myUsageFlags & (uint)GpuBufferUsage::CONSTANT_BUFFER) != 0;

    D3D12_HEAP_PROPERTIES heapProps;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.CreationNodeMask = 1u;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.VisibleNodeMask = 1u;

    myAlignment = 0u;
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
    resourceDesc.Flags = wantsUnorderedAccess ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

    const bool cpu_write = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::WRITE) > 0u;
    const bool cpu_read = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::READ) > 0u;

    // Unused and not needed in D3D12?
    // const bool coherent = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::COHERENT) > 0u;
    // const bool dynamic = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::DYNAMIC) > 0u;

    myUsageState = GpuResourceState::RESOURCE_STATE_COMMON;
    if (!cpu_write && !cpu_read)  // No CPU-access
    {
      heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
      myUsageState = GpuResourceState::RESOURCE_STATE_COMMON;
    }
    else if (cpu_write && !cpu_read)  // CPU write-only
    {
      heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
      myUsageState = GpuResourceState::RESOURCE_STATE_GENERIC_READ;
    }
    else if (cpu_read && !cpu_write)  // CPU read-only
    {
      heapProps.Type = D3D12_HEAP_TYPE_READBACK;
      myUsageState = GpuResourceState::RESOURCE_STATE_COPY_DEST;
    }
    else if (cpu_read && cpu_write) 
    {
      // TODO: This might be wrong and is a bad idea anyway... 
      heapProps.Type = D3D12_HEAP_TYPE_CUSTOM;
      myUsageState = GpuResourceState::RESOURCE_STATE_GENERIC_READ;
      heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
      heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    }

    RenderCore_PlatformDX12* dx12Platform = RenderCore::GetPlatformDX12();
    D3D12_RESOURCE_STATES usageStateDX12 = Adapter::toNativeType(myUsageState);

    CheckD3Dcall(dx12Platform->GetDevice()->CreateCommittedResource(
      &heapProps, 
      D3D12_HEAP_FLAG_NONE, 
      &resourceDesc, 
      usageStateDX12,
      nullptr, IID_PPV_ARGS(&storageDx12->myResource)));

    if (pInitialData != nullptr)
    {
      if (cpu_write)  // The fast path: Just lock and memcpy into cpu-visible region
      {
        void* dest = Lock(GpuResoruceLockOption::WRITE);
        ASSERT(dest != nullptr);
        memcpy(dest, pInitialData, someParameters.uNumElements * someParameters.uElementSizeBytes);
        Unlock();
      }
      else
      {
        RenderCore::InitBufferData(this, pInitialData);
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

      if (wantsUnorderedAccess)
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
    myStorage = nullptr;
    memset(&myVertexBufferView, 0, sizeof(myVertexBufferView));
    memset(&myIndexBufferView, 0, sizeof(myIndexBufferView));
  }
//---------------------------------------------------------------------------//
  void* GpuBufferDX12::Lock(GpuResoruceLockOption eLockOption, uint uOffsetElements /* = 0u */, uint uNumElements /* = 0u */)
  {
    ASSERT(uOffsetElements + uNumElements <= myParameters.uNumElements);

    if (uNumElements == 0u)
      uNumElements = myParameters.uNumElements - uOffsetElements;

    D3D12_RANGE range;
    range.Begin = uOffsetElements * myParameters.uElementSizeBytes;
    range.End = range.Begin + uNumElements * myParameters.uElementSizeBytes;

    const bool isCpuWritable = (myParameters.uAccessFlags & (uint)GpuResourceAccessFlags::WRITE) > 0u;
    const bool isCpuReadable = (myParameters.uAccessFlags & (uint)GpuResourceAccessFlags::READ) > 0u;

    const bool wantsWrite = eLockOption == GpuResoruceLockOption::READ_WRITE || eLockOption == GpuResoruceLockOption::WRITE;
    const bool wantsRead = eLockOption == GpuResoruceLockOption::READ_WRITE || eLockOption == GpuResoruceLockOption::READ;

    if ((wantsWrite && !isCpuWritable) || (wantsRead && !isCpuReadable))
      return nullptr;

    // TODO: Do something with the current usage type? Transition it into something correct? Early-out?

    GpuResourceStorageDX12* storageDx12 = static_cast<GpuResourceStorageDX12*>(myStorage.get());

    void* mappedData = nullptr;
    CheckD3Dcall(storageDx12->myResource->Map(0, &range, &mappedData));

    return mappedData;
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::Unlock(uint anOffsetElements /* = 0u */, uint aNumElements /* = 0u */)
  {
    if (anOffsetElements == 0u && aNumElements == 0u)
      aNumElements = myParameters.uNumElements;

    ASSERT(anOffsetElements + aNumElements <= myParameters.uNumElements);

    D3D12_RANGE range;
    range.Begin = anOffsetElements * myParameters.uElementSizeBytes;
    range.End = range.Begin + aNumElements * myParameters.uElementSizeBytes;

    GpuResourceStorageDX12* storageDx12 = static_cast<GpuResourceStorageDX12*>(myStorage.get());
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
