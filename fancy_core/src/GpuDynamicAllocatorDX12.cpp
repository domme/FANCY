#include "FancyCorePrerequisites.h"

#include "GpuDynamicAllocatorDX12.h"

#if defined (RENDERER_DX12)
#include "Renderer.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  GpuDynamicAllocPage::GpuDynamicAllocPage(ID3D12Resource* aResource, D3D12_RESOURCE_STATES aDefaultUsage, bool aCpuAccessRequired)
    : myCpuDataPtr(nullptr)
  {
    myResource.Attach(aResource);
    myUsageState = aDefaultUsage;
    if (aCpuAccessRequired)
      CheckD3Dcall(myResource->Map(0, nullptr, &myCpuDataPtr));  // Map the whole range (nullptr)
  }
//---------------------------------------------------------------------------//
  GpuDynamicAllocPage::~GpuDynamicAllocPage()
  {
    if (myCpuDataPtr != nullptr)
      myResource->Unmap(0u, nullptr);
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  GpuDynamicAllocatorDX12::GpuDynamicAllocatorDX12(RenderOutputDX12& aRenderer, GpuDynamicAllocatorType aType)
    : myCurrPage(nullptr)
    , myCurrPageOffsetBytes(0u)
    , myType(aType)
    , myRenderer(aRenderer)
  {
    
  }
//---------------------------------------------------------------------------//
  GpuDynamicAllocatorDX12::~GpuDynamicAllocatorDX12()
  {
    // If there are still pages in this list, we destroyed the allocator too early...
    ASSERT(myFullyUsedPages.empty(), "There are still pages in use by a command list. The allocator has to be destroyed only after the last frame is finished");

    delete myCurrPage;

    for (GpuDynamicAllocPage* page : myAvailablePages)
      delete page;

    myAvailablePages.clear();
    
    for (auto waitingEntry : myWaitingPages)
    {
      ASSERT(myRenderer.IsFenceDone(waitingEntry.first));
      delete waitingEntry.second;
    }
  }
//---------------------------------------------------------------------------//
  AllocResult GpuDynamicAllocatorDX12::Allocate(size_t aSizeBytes, size_t anAlignment)
  {
    // 0) Check for argument-sanity
    const uint32 pageSize = myType == GpuDynamicAllocatorType::GpuOnly ? kGpuPageSize : kCpuPageSize;
    ASSERT(aSizeBytes <= pageSize);

    // 1) Check if some waiting pages can be made available
    while (!myWaitingPages.empty())
    {
      auto& waitingEntry = myWaitingPages.front();
      if (myRenderer.IsFenceDone(waitingEntry.first))
      {
        myAvailablePages.push_back(waitingEntry.second);
        myWaitingPages.pop_front();
      }
      else
      {
        // All "newer" entries also won't be done yet, so we can just break here.
        break;
      }
    }

    // 2) Mark the current page as fully used if we can't allocate from it anymore
    if (myCurrPage != nullptr && myCurrPageOffsetBytes + aSizeBytes > pageSize)
    {
      myFullyUsedPages.push_back(myCurrPage);
      myCurrPage = nullptr;
      myCurrPageOffsetBytes = 0u;
    }
    
    if (myCurrPage == nullptr)
    {
      // 3) Can we pick a page from the available pages?
      if (!myAvailablePages.empty())
      {
        myCurrPage = myAvailablePages.front();
        myAvailablePages.pop_front();
      }
      // 4) Create a completely new page if none available
      else
      {
        myCurrPage = CreateNewPage();
      }
    }
    
    // 5) Finally assemble and return the AllocResult
    const uint32 offset = myCurrPageOffsetBytes;
    myCurrPageOffsetBytes += aSizeBytes;

    AllocResult result;
    result.myCpuDataPtr = static_cast<uint8*>(myCurrPage->myCpuDataPtr) + offset;
    result.myResource = myCurrPage;
    result.myGpuVirtualOffset = offset;
    result.mySize = aSizeBytes;

    return result;
  }
//---------------------------------------------------------------------------//
  GpuDynamicAllocPage* GpuDynamicAllocatorDX12::CreateNewPage()
  {
    D3D12_HEAP_PROPERTIES heapProperties;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.CreationNodeMask = 1;
    heapProperties.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resourceDesc;
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    D3D12_RESOURCE_STATES initialResourceState;

    if (myType == GpuDynamicAllocatorType::GpuOnly)
    {
      heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
      resourceDesc.Width = kGpuPageSize;

      // Gpu-Only pages that are created with this allocator will most likely be used as UAVs
      // because this allocator only creates temporary pages valid for one frame.
      resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
      initialResourceState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    }
    else
    {
      heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
      resourceDesc.Width = kCpuPageSize;
      resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
      initialResourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
    }

    ID3D12Resource* resource;

    CheckD3Dcall(
      myRenderer.GetDevice()->CreateCommittedResource(
        &heapProperties, 
        D3D12_HEAP_FLAG_NONE, 
        &resourceDesc, 
        initialResourceState, 
        nullptr, 
        IID_PPV_ARGS(&resource))
      );

    return new GpuDynamicAllocPage(resource, initialResourceState, myType == GpuDynamicAllocatorType::CpuWritable);
  }
//---------------------------------------------------------------------------//
  void GpuDynamicAllocatorDX12::CleanupAfterCmdListExecute(uint64 aCmdListDoneFence)
  {
    // This method should be called after the owning command list is launched for execution. At this point we transfer all used pages to the waiting pages queue
    while(!myFullyUsedPages.empty())
    {
      myWaitingPages.push_back(std::make_pair(aCmdListDoneFence, myFullyUsedPages.front()));
      myFullyUsedPages.pop_front();
    }
  }
//---------------------------------------------------------------------------//
} } }

#endif  // RENDERER_DX12
  