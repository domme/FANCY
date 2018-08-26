#include "GpuBufferDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"
#include "AdapterDX12.h"
#include "GpuResourceStorageDX12.h"

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
  void GpuBufferDX12::Create(const GpuBufferProperties& someProperties, const void* pInitialData)
  {
    ASSERT(someProperties.myElementSizeBytes > 0 && someProperties.myNumElements > 0, "Invalid buffer size specified");

    Destroy();

    GpuResourceStorageDX12* storageDx12 = new GpuResourceStorageDX12();
    myStorage.reset(storageDx12);

    myProperties = someProperties;

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

    myUsageState = GpuResourceState::RESOURCE_STATE_COMMON;

    const GpuMemoryAccessType gpuMemAccess = (GpuMemoryAccessType)someProperties.myCpuAccess;
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
    D3D12_RESOURCE_STATES usageStateDX12 = Adapter::ResolveResourceState(myUsageState);
    ID3D12Device* device = dx12Platform->GetDevice();
        
    GpuMemoryAllocationDX12 gpuMemory = dx12Platform->AllocateGpuMemory(GpuMemoryType::BUFFER, gpuMemAccess, pitch, myAlignment);
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
      RenderCore::GetPlatformDX12()->FreeGpuMemory(storageDx12->myGpuMemory);
    }

    myStorage = nullptr;
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
