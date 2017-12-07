#include "GpuBufferDX12.h"
#include "Fancy.h"
#include "DescriptorHeapPoolDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"
#include "AdapterDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {
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
  void GpuBufferDX12::Create(const GpuBufferCreationParams& someParameters, void* pInitialData)
  {
    Destroy();

    ASSERT(someParameters.uElementSizeBytes > 0 && someParameters.uNumElements > 0,
      "Invalid buffer size specified");

    GpuBufferCreationParams* pBaseParams = &myParameters;
    *pBaseParams = someParameters;
    
    const bool wantsUnorderedAccess = 
      (someParameters.myUsageFlags & (uint32)GpuBufferUsage::RESOURCE_BUFFER_RW) 
      || (someParameters.myUsageFlags & (uint32)GpuBufferUsage::RESOURCE_BUFFER_LARGE_RW);

    const bool wantsShaderResourceView =
      wantsUnorderedAccess 
      || (someParameters.myUsageFlags & (uint32)GpuBufferUsage::RESOURCE_BUFFER)
      || (someParameters.myUsageFlags & (uint32)GpuBufferUsage::RESOURCE_BUFFER_LARGE);

    const bool wantsVboView =
      (someParameters.myUsageFlags & (uint32)GpuBufferUsage::VERTEX_BUFFER) != 0;

    const bool wantsIboView =
      (someParameters.myUsageFlags & (uint32)GpuBufferUsage::INDEX_BUFFER) != 0;

    const bool wantsConstantBufferView =
      (someParameters.myUsageFlags & (uint32)GpuBufferUsage::CONSTANT_BUFFER) != 0;

    D3D12_HEAP_PROPERTIES heapProps;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.CreationNodeMask = 1u;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.VisibleNodeMask = 1u;

    myAlignment = 0u;
    if (wantsConstantBufferView)
      myAlignment = 256u;

    uint32 actualWidthBytesWithAlignment = 
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

    const bool wantsCpuWrite = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::WRITE) > 0u;
    const bool wantsCpuRead = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::READ) > 0u;
    const bool wantsCpuStorage = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::PREFER_CPU_STORAGE) > 0u;
    const bool wantsCoherent = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::COHERENT) > 0u;

    // TODO: What to do with these flags in DX12?
    //const bool wantsDynamic = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::DYNAMIC) > 0u;

    myUsageState = GpuResourceState::RESOURCE_STATE_COMMON;
    if (!wantsCpuWrite && !wantsCpuRead && !wantsCpuStorage)
    {
      // The default for most buffers: No Cpu-access at all required. Can be created as GPU-only visible heap
      heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
      myUsageState = GpuResourceState::RESOURCE_STATE_COMMON;
    }
    else if (wantsCpuWrite || wantsCpuStorage)
    {
      heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
      myUsageState = GpuResourceState::RESOURCE_STATE_GENERIC_READ;
      
      if (wantsCpuRead)
        heapProps.CPUPageProperty = wantsCoherent ? D3D12_CPU_PAGE_PROPERTY_WRITE_BACK : D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
    }
    else if (!wantsCpuWrite && wantsCpuRead)
    {
      heapProps.Type = D3D12_HEAP_TYPE_READBACK;
      heapProps.CPUPageProperty = wantsCoherent ? D3D12_CPU_PAGE_PROPERTY_WRITE_BACK : D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
      myUsageState = GpuResourceState::RESOURCE_STATE_COPY_DEST;
    }

    RenderCore_PlatformDX12* dx12Platform = RenderCore::GetPlatformDX12();
    D3D12_RESOURCE_STATES usageStateDX12 = Adapter::toNativeType(myUsageState);

    CheckD3Dcall(dx12Platform->GetDevice()->CreateCommittedResource(
      &heapProps, 
      D3D12_HEAP_FLAG_NONE, 
      &resourceDesc, 
      usageStateDX12,
      nullptr, IID_PPV_ARGS(&myResource)));

    // Create derived views
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
      dx12Platform->GetDevice()->CreateShaderResourceView(myResource.Get(), &srvDesc, mySrvDescriptor.myCpuHandle);
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
      dx12Platform->GetDevice()->CreateUnorderedAccessView(myResource.Get(), nullptr, &uavDesc, myUavDescriptor.myCpuHandle);
    }

    if (wantsConstantBufferView)
    {
      D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
      cbvDesc.SizeInBytes = actualWidthBytesWithAlignment;
      cbvDesc.BufferLocation = GetGpuVirtualAddress();

      myCbvDescriptor = dx12Platform->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
      dx12Platform->GetDevice()->CreateConstantBufferView(&cbvDesc, myCbvDescriptor.myCpuHandle);
    }

    if (wantsVboView)
    {
      myVertexBufferView.BufferLocation = GetGpuVirtualAddress();
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

      myIndexBufferView.BufferLocation = GetGpuVirtualAddress();
      myIndexBufferView.SizeInBytes = myParameters.uNumElements * myParameters.uElementSizeBytes;
    }
        
    if (pInitialData != nullptr)
    {
      if (wantsCpuWrite)  // The fast path: Just lock and memcpy into cpu-visible region
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
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::Destroy()
  {
    GpuResourceDX12::Reset();
    memset(&myVertexBufferView, 0, sizeof(myVertexBufferView));
    memset(&myIndexBufferView, 0, sizeof(myIndexBufferView));
  }
//---------------------------------------------------------------------------//
  void locResolveLockOptions(GpuResoruceLockOption aLockOption, bool& aNeedsRead, bool& aNeedsWrite, bool& aNeedsRename)
  {
    aNeedsRead = false;
    aNeedsWrite = false;
    aNeedsRename = false;

    switch (aLockOption)
    {
      case GpuResoruceLockOption::READ: 
      case GpuResoruceLockOption::READ_UNSYNCHRONIZED:
      case GpuResoruceLockOption::READ_PERSISTENT:
      case GpuResoruceLockOption::READ_PERSISTENT_COHERENT:
        aNeedsRead = true;
        break;
      case GpuResoruceLockOption::WRITE: 
      case GpuResoruceLockOption::WRITE_UNSYNCHRONIZED:
      case GpuResoruceLockOption::WRITE_PERSISTENT:
      case GpuResoruceLockOption::WRITE_PERSISTENT_COHERENT:
        aNeedsWrite = true;
        break;
      case GpuResoruceLockOption::READ_WRITE: 
      case GpuResoruceLockOption::READ_WRITE_UNSYNCHRONIZED:
      case GpuResoruceLockOption::READ_WRITE_PERSISTENT:
      case GpuResoruceLockOption::READ_WRITE_PERSISTENT_COHERENT:
        aNeedsRead = true;
        aNeedsWrite = true;
        break;
      case GpuResoruceLockOption::WRITE_DISCARD: 
        aNeedsWrite = true;
        aNeedsRename = true;
        break;
      case GpuResoruceLockOption::READ_WRITE_DISCARD: 
        aNeedsRead = true;
        aNeedsWrite = true;
        aNeedsRename = true;
        break;
    }
  }
//---------------------------------------------------------------------------//
  void* GpuBufferDX12::Lock(GpuResoruceLockOption eLockOption, uint uOffsetElements, uint uNumElements)
  {
    if (myState.isLocked)
      return nullptr;

    bool needsRead, needsWrite, needsRename;
    locResolveLockOptions(eLockOption, needsRead, needsWrite, needsRename);
   
    D3D12_RANGE range;
    range.Begin = uOffsetElements * myParameters.uElementSizeBytes;
    range.End = range.Begin + uNumElements * myParameters.uElementSizeBytes;

    const bool isCpuWritable = (myParameters.uAccessFlags & (uint)GpuResourceAccessFlags::WRITE) > 0u;
    const bool isCpuReadable = (myParameters.uAccessFlags & (uint)GpuResourceAccessFlags::READ) > 0u;

    if ((needsWrite && !isCpuWritable) || (needsRead && !isCpuReadable))
      return nullptr;

    // TODO: Do something with the current usage type? Transition it into something correct? Early-out?

    void* mappedData;
    CheckD3Dcall(myResource->Map(0, &range, &mappedData));

    if (mappedData != nullptr)
    {
      myState.isLocked = true;
      myState.myLockedRange_Begin = range.Begin;
      myState.myLockedRange_End = range.End;
      myState.myCachedLockDataPtr = mappedData;
    }
    
    return mappedData;
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::Unlock()
  {
    if (!myState.isLocked)
      return;

    D3D12_RANGE range;
    range.Begin = myState.myLockedRange_Begin;
    range.End = myState.myLockedRange_End;

    myResource->Unmap(0u, &range);

    myState.isLocked = false;
    myState.myLockedRange_Begin = 0u;
    myState.myLockedRange_End = 0u;
    myState.myCachedLockDataPtr = nullptr;
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
} } }
