#include "fancy_core_precompile.h"
#include "GpuMemoryAllocatorDX12.h"

#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"
#include "AdapterDX12.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  using Page = PagedLinearAllocator<Microsoft::WRL::ComPtr<ID3D12Heap>>::Page;
  using Block = PagedLinearAllocator<Microsoft::WRL::ComPtr<ID3D12Heap>>::Block;
//---------------------------------------------------------------------------//
  namespace Priv_GpuMemoryAllocatorDX12
  {
    bool locCreateHeap(GpuMemoryType aType, CpuMemoryAccessType anAccessType, uint64 aSize, Microsoft::WRL::ComPtr<ID3D12Heap>& aHeapOut)
    {
      ID3D12Device* device = RenderCore::GetPlatformDX12()->GetDevice();
      const uint64 alignedSize  = MathUtil::Align(aSize, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);

      D3D12_HEAP_DESC heapDesc{ 0u };
      heapDesc.SizeInBytes = alignedSize;
      heapDesc.Flags = Adapter::ResolveHeapFlags(aType);
      heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
      heapDesc.Properties.Type = RenderCore_PlatformDX12::ResolveHeapType(anAccessType);
      heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

      Microsoft::WRL::ComPtr<ID3D12Heap> heap;
      if (!SUCCEEDED(device->CreateHeap(&heapDesc, IID_PPV_ARGS(&heap))))
        return false;

      aHeapOut = heap;
      return true;
    }
//---------------------------------------------------------------------------//
    void locDestroyHeap(Microsoft::WRL::ComPtr<ID3D12Heap>& /*aHeap*/)
    {
      // No special treatment needed here. Resource will be released by the smartptr
    }
//---------------------------------------------------------------------------//
    const char* locMemoryTypeToString(GpuMemoryType aType)
    {
      switch(aType) 
      { 
      case GpuMemoryType::BUFFER: return "buffer";
      case GpuMemoryType::TEXTURE: return "texture";
      case GpuMemoryType::RENDERTARGET: return "rendertarget";
        default: return "";
      }
    }
//---------------------------------------------------------------------------//
    const char* locCpuAccessTypeToString(CpuMemoryAccessType aType)
    {
      switch(aType) 
      { 
      case CpuMemoryAccessType::NO_CPU_ACCESS: return "default";
      case CpuMemoryAccessType::CPU_WRITE: return "write";
      case CpuMemoryAccessType::CPU_READ: return "read";
        default: return "";
      }
    }
//---------------------------------------------------------------------------//
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//  
  GpuMemoryAllocatorDX12::GpuMemoryAllocatorDX12(GpuMemoryType aType, CpuMemoryAccessType anAccessType, uint64 aMemBlockSize)
    : myType(aType)
    , myAccess(anAccessType)
    , myAllocator(aMemBlockSize, 
      [aType, anAccessType](uint64 aSize, Microsoft::WRL::ComPtr<ID3D12Heap>& aHeapOut){return Priv_GpuMemoryAllocatorDX12::locCreateHeap(aType, anAccessType, aSize, aHeapOut); }
      , Priv_GpuMemoryAllocatorDX12::locDestroyHeap)
  {
  }
//---------------------------------------------------------------------------//
  GpuMemoryAllocatorDX12::~GpuMemoryAllocatorDX12()
  {
#if FANCY_RENDERER_DEBUG_MEMORY_ALLOCS
    for (auto it : myAllocDebugInfos)
      LOG_WARNING("Leaked GPU memory allocation: % with block (start: %, end: %)", it.myName.c_str(), it.myStart, it.myEnd);  
#endif

    ASSERT(myAllocator.IsEmpty(), "There are still gpu-resources allocated when destroying the memory allocator");
  }
//---------------------------------------------------------------------------//
  GpuMemoryAllocationDX12 GpuMemoryAllocatorDX12::Allocate(const uint64 aSize, const uint anAlignment, const char* aDebugName /*= nullptr*/)
  {
    if (myType == GpuMemoryType::BUFFER && myAccess == CpuMemoryAccessType::NO_CPU_ACCESS)
    {
      LOG_DEBUG("");
      LOG_DEBUG("Allocate % bytes from allocator % - % (% Byte page size):", MathUtil::Align(aSize, anAlignment), Priv_GpuMemoryAllocatorDX12::locMemoryTypeToString(myType), Priv_GpuMemoryAllocatorDX12::locCpuAccessTypeToString(myAccess), myAllocator.myPageSize);
      LOG_DEBUG("Before:");
      DebugPrint();
    }

    uint64 offsetInPage;
    const Page* page = myAllocator.Allocate(aSize, anAlignment, offsetInPage);
    if (page == nullptr)
      return GpuMemoryAllocationDX12{};
    
    GpuMemoryAllocationDX12 allocResult;
    allocResult.myOffsetInHeap = offsetInPage;
    allocResult.mySize = aSize;
    allocResult.myHeap = page->myData.Get();

#if FANCY_RENDERER_DEBUG_MEMORY_ALLOCS
    AllocDebugInfo debugInfo;
    debugInfo.myName = aDebugName != nullptr ? aDebugName : "Unnamed GPU memory allocation";
    debugInfo.myStart = page->myStart + offsetInPage;
    debugInfo.myEnd = debugInfo.myStart + MathUtil::Align(aSize, anAlignment);
    myAllocDebugInfos.push_back(debugInfo);
#endif

    if (myType == GpuMemoryType::BUFFER && myAccess == CpuMemoryAccessType::NO_CPU_ACCESS)
    {
      LOG_DEBUG("After:");
      DebugPrint();
      LOG_DEBUG("");
    }

    return allocResult;
  }
//---------------------------------------------------------------------------//
  void GpuMemoryAllocatorDX12::Free(GpuMemoryAllocationDX12& anAllocation)
  {
    const Page* page = myAllocator.FindPage([&](const Page& aPage) {
      return anAllocation.myHeap == aPage.myData.Get();
    });

    ASSERT(page != nullptr);

    Block block;
    block.myStart = page->myStart + anAllocation.myOffsetInHeap;
    block.myEnd = block.myStart + anAllocation.mySize;

    if (myType == GpuMemoryType::BUFFER && myAccess == CpuMemoryAccessType::NO_CPU_ACCESS)
    {
      LOG_DEBUG("");
      LOG_DEBUG("Free block (%, %) from allocator % - % (% Byte page size):", block.myStart, block.myEnd, Priv_GpuMemoryAllocatorDX12::locMemoryTypeToString(myType), Priv_GpuMemoryAllocatorDX12::locCpuAccessTypeToString(myAccess), myAllocator.myPageSize);
      LOG_DEBUG("Before: ");
      DebugPrint();
    }

    myAllocator.Free(block);

#if FANCY_RENDERER_DEBUG_MEMORY_ALLOCS
    auto it = std::find_if(myAllocDebugInfos.begin(), myAllocDebugInfos.end(), [&block](const AllocDebugInfo& anInfo)
    {
      return anInfo.myStart == block.myStart;
    });

    ASSERT(it != myAllocDebugInfos.end());
    myAllocDebugInfos.erase(it);

    if (myType == GpuMemoryType::BUFFER && myAccess == CpuMemoryAccessType::NO_CPU_ACCESS)
    {
      LOG_DEBUG("After:");
      DebugPrint();
      LOG_DEBUG("");
    }
#endif
  }

  void GpuMemoryAllocatorDX12::DebugPrint()
  {
    std::stringstream debugStr;
    debugStr << "Num Pages: " << myAllocator.myPages.size() << std::endl;
    debugStr << "Free list: " << std::endl;
    int oldPageIdx = -2;
    uint64 lastElementEnd = 0;
    for (auto it = myAllocator.myFreeList.Begin(); it != myAllocator.myFreeList.Invalid(); ++it)
    {
      int pageIdx = -1;
      for (int i = 0; i < myAllocator.myPages.size(); ++i)
      {
        if (myAllocator.IsBlockInPage(*it, myAllocator.myPages[i]))
        {
          pageIdx = i;
          break;
        }
      }

      if (oldPageIdx != pageIdx)
        debugStr << "|| Page " << pageIdx << ": ";
      oldPageIdx = pageIdx;

      if (lastElementEnd != it->myStart)
        debugStr << "[X: " << (it->myStart - lastElementEnd) << "]";
      lastElementEnd = it->myEnd;

      debugStr << "[" << it->myStart << ".." << it->myEnd << "]";
    }

    LOG_DEBUG("%", debugStr.str().c_str());
  }
//---------------------------------------------------------------------------//
}

