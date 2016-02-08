#include "GpuBufferDX12.h"
#include "Renderer.h"

#if defined (RENDERER_DX12)

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  GpuBufferDX12::GpuBufferDX12()
  {
  }
//---------------------------------------------------------------------------//
  GpuBufferDX12::~GpuBufferDX12()
  {
  }
//---------------------------------------------------------------------------//
  bool GpuBufferDX12::operator==(const GpuBufferDesc& aDesc) const
  {
    return GetDescription() == aDesc;
  }
//---------------------------------------------------------------------------//
  GpuBufferDesc GpuBufferDX12::GetDescription() const
  {
    GpuBufferDesc desc;
    desc.myInternalRefIndex = myParameters.myInternalRefIndex;
    return desc;
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::setBufferData(void* pData, uint uOffsetElements, uint uNumElements)
  {
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::create(const GpuBufferCreationParams& someParameters, void* pInitialData)
  {
    destroy();

    RendererDX12& renderer = Renderer::getInstance();

    ASSERT_M(someParameters.uElementSizeBytes > 0 && someParameters.uNumElements > 0,
      "Invalid buffer size specified");

    GpuBufferCreationParams* pBaseParams = &myParameters;
    *pBaseParams = someParameters;
    
    D3D12_RESOURCE_STATES resourceStates = D3D12_RESOURCE_STATE_COMMON;
    
    switch (someParameters.ePrimaryUsageType)
    {
      case GpuBufferUsage::CONSTANT_BUFFER: 
      case GpuBufferUsage::VERTEX_BUFFER:
        resourceStates |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        break;
      case GpuBufferUsage::INDEX_BUFFER: 
        resourceStates |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
        break;
      case GpuBufferUsage::DRAW_INDIRECT_BUFFER: 
      case GpuBufferUsage::DISPATCH_INDIRECT_BUFFER:
        resourceStates |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
        break;
      case GpuBufferUsage::RESOURCE_BUFFER: 
      case GpuBufferUsage::RESOURCE_BUFFER_LARGE:
        resourceStates |= 
          D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        break;
      case GpuBufferUsage::RESOURCE_BUFFER_RW:
      case GpuBufferUsage::RESOURCE_BUFFER_LARGE_RW:
        resourceStates |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
       break;
      default: break;
    }
    
    const bool wantsUnorderedAccess = 
    someParameters.ePrimaryUsageType == GpuBufferUsage::RESOURCE_BUFFER_RW 
    || someParameters.ePrimaryUsageType == GpuBufferUsage::RESOURCE_BUFFER_LARGE_RW;

    D3D12_HEAP_PROPERTIES heapProps;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
    heapProps.CreationNodeMask = 1u;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.VisibleNodeMask = 1u;

    D3D12_RESOURCE_DESC resourceDesc;
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = someParameters.uNumElements * someParameters.uElementSizeBytes;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = wantsUnorderedAccess ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

    const bool wantsCpuWrite = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::WRITE) > 0u;
    const bool wantsCpuRead = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::READ) > 0u;
    const bool wantsCpuStorage = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::PREFER_CPU_STORAGE) > 0u;
    const bool wantsCoherent = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::COHERENT) > 0u;

    // TODO: What to do with these flags in DX12?
    //const bool wantsDynamic = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::DYNAMIC) > 0u;

    if (!wantsCpuWrite && !wantsCpuRead && !wantsCpuStorage)
    {
      // The default for most buffers: No Cpu-access at all required. Can be created as GPU-only visible heap
      heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
      heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
      myUsageState = D3D12_RESOURCE_STATE_COMMON;
    }
    else if (wantsCpuWrite || wantsCpuStorage)
    {
      heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
      myUsageState = D3D12_RESOURCE_STATE_GENERIC_READ;

      if (!wantsCpuWrite && !wantsCpuRead)
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
      else if (wantsCpuRead)
        heapProps.CPUPageProperty = wantsCoherent ? D3D12_CPU_PAGE_PROPERTY_WRITE_BACK : D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
    }
    else if (!wantsCpuWrite && wantsCpuRead)
    {
      heapProps.Type = D3D12_HEAP_TYPE_READBACK;
      heapProps.CPUPageProperty = wantsCoherent ? D3D12_CPU_PAGE_PROPERTY_WRITE_BACK : D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
      myUsageState = D3D12_RESOURCE_STATE_COPY_DEST;
    }

    CheckD3Dcall(renderer.GetDevice()->CreateCommittedResource(
      &heapProps, 
      D3D12_HEAP_FLAG_NONE, 
      &resourceDesc, 
      resourceStates, 
      nullptr, IID_PPV_ARGS(&myResource)));

    if (pInitialData != nullptr)
    {
      if (wantsCpuWrite)  // The fast path: Just lock and memcpy into cpu-visible region
      {
        void* dest = lock(GpuResoruceLockOption::WRITE);
        ASSERT(dest != nullptr);
        memcpy(dest, pInitialData, someParameters.uNumElements * someParameters.uElementSizeBytes);
        unlock();
      }
      else
      {
        renderer.InitBufferData(this, pInitialData);
      }
    }
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::destroy()
  {
    myResource = nullptr;
    myUsageState = static_cast<D3D12_RESOURCE_STATES>(~0);
    myTransitioningState = static_cast<D3D12_RESOURCE_STATES>(~0);
    myGpuVirtualAddress = 0u;
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
  void* GpuBufferDX12::lock(GpuResoruceLockOption eLockOption, uint uOffsetElements, uint uNumElements)
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
      myState.myLockedRange = range;
      myState.myCachedLockDataPtr = mappedData;
    }
    
    return mappedData;
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::unlock()
  {
    if (!myState.isLocked)
      return;

    myResource->Unmap(0u, &myState.myLockedRange);

    myState.isLocked = false;
    myState.myLockedRange = { 0 };
    myState.myCachedLockDataPtr = nullptr;
  }
//---------------------------------------------------------------------------//
} } }

#endif
