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

    // Create the default resource (no CPU-access)
    {
      D3D12_HEAP_PROPERTIES heapProps;
      heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
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

      ASSERT(renderer.GetDevice()->CreateCommittedResource(
        &heapProps, 
        D3D12_HEAP_FLAG_NONE, 
        &resourceDesc, 
        resourceStates, 
        nullptr, IID_PPV_ARGS(&myResource)));
    }

    // Do we need additional resources for CPU-access?
    const bool wantsCpuWrite = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::WRITE) > 0u;
    const bool wantsCpuRead = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::READ) > 0u;

    // Can we do something with these flags on DX12?
    //const bool wantsDynamic = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::DYNAMIC) > 0u;
    //const bool wantsCoherent = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::COHERENT) > 0u;
    //const bool wantsCpuStorage = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::PREFER_CPU_STORAGE) > 0u;

    if (wantsCpuWrite)
    {
      D3D12_HEAP_PROPERTIES heapProps;
      heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
      heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
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
      resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

      ASSERT(renderer.GetDevice()->CreateCommittedResource(
        &heapProps, 
        D3D12_HEAP_FLAG_NONE, 
        &resourceDesc, 
        D3D12_RESOURCE_STATE_GENERIC_READ, 
        nullptr, IID_PPV_ARGS(&myUploadResource)));
    }

    if (wantsCpuRead)
    {
      D3D12_HEAP_PROPERTIES heapProps;
      heapProps.Type = D3D12_HEAP_TYPE_READBACK;
      heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
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
      resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

      ASSERT(renderer.GetDevice()->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr, IID_PPV_ARGS(&myDownloadResource)));
    }

    if (pInitialData != nullptr)
    {
      void* dest = lock(GpuResoruceLockOption::WRITE);
      ASSERT(dest != nullptr);
      memcpy(dest, pInitialData, someParameters.uNumElements * someParameters.uElementSizeBytes);
      unlock();
    }
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::destroy()
  {
    myResource = nullptr;
    myDownloadResource = nullptr;
    myUploadResource = nullptr;
  }
//---------------------------------------------------------------------------//
  void* GpuBufferDX12::lock(GpuResoruceLockOption eLockOption, uint uOffsetElements, uint uNumElements)
  {

    return nullptr;
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::unlock()
  {
      
  }
//---------------------------------------------------------------------------//
} } }

#endif
