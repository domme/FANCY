#include "fancy_core_precompile.h"
#include "ShaderVisibleDescriptorHeapDX12.h"

#include "DescriptorDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"
#include "TimeManager.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
//---------------------------------------------------------------------------//
  namespace Private
  {
    static D3D12_DESCRIPTOR_RANGE_TYPE GetDescriptorRangeType(BindlessDescriptorType aBindlessType)
    {
      switch(aBindlessType)
      {
      case BINDLESS_DESCRIPTOR_TEXTURE_2D: return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
      case BINDLESS_DESCRIPTOR_RW_TEXTURE_2D: return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
      case BINDLESS_DESCRIPTOR_BUFFER: return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
      case BINDLESS_DESCRIPTOR_RW_BUFFER: return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
      case BINDLESS_DESCRIPTOR_SAMPLER: return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
      case BINDLESS_DESCRIPTOR_NUM: ASSERT(false); return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
      }
    }

    static GpuResourceDimension GetResourceDimension(BindlessDescriptorType aBindlessType)
    {
      GpuResourceDimension resourceDimension = GpuResourceDimension::UNKONWN;
      switch (aBindlessType)
      {
      case BINDLESS_DESCRIPTOR_TEXTURE_2D:
      case BINDLESS_DESCRIPTOR_RW_TEXTURE_2D:
        return GpuResourceDimension::TEXTURE_2D;
      case BINDLESS_DESCRIPTOR_BUFFER:
      case BINDLESS_DESCRIPTOR_RW_BUFFER:
        return GpuResourceDimension::BUFFER;
      case BINDLESS_DESCRIPTOR_SAMPLER:
        return GpuResourceDimension::UNKONWN;
      default: ASSERT(false); return GpuResourceDimension::TEXTURE_2D;
      }
    }
  }
//---------------------------------------------------------------------------//
  ShaderVisibleDescriptorHeapDX12::ShaderVisibleDescriptorHeapDX12(uint aNumBindlessTextures, uint aNumBindlessRWTextures, uint aNumBindlessBuffers, uint aNumBindlessRWBuffers,
    uint aNumTransientDescriptors, uint aNumTransientDescriptorsPerRange)
    : myOverallNumBindlessDescriptors(aNumBindlessTextures + aNumBindlessBuffers + aNumBindlessRWTextures + aNumBindlessRWBuffers)
    , myNumBindlessDescriptors{ aNumBindlessTextures, aNumBindlessRWTextures, aNumBindlessBuffers, aNumBindlessRWBuffers, 0 }
    , myNumTransientDescriptors(aNumTransientDescriptors)
    , myNumTransientDescriptorsPerRange(aNumTransientDescriptorsPerRange)
    , myNumTransientRanges(aNumTransientDescriptors / aNumTransientDescriptorsPerRange)
    , myBindlessAllocators{
      PagedLinearAllocator(aNumBindlessTextures),
      PagedLinearAllocator(aNumBindlessRWTextures),
      PagedLinearAllocator(aNumBindlessBuffers),
      PagedLinearAllocator(aNumBindlessRWBuffers),
      PagedLinearAllocator(0)
    }
  {
    ASSERT(aNumTransientDescriptorsPerRange <= aNumTransientDescriptors);

    Init(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  }
//---------------------------------------------------------------------------//
  ShaderVisibleDescriptorHeapDX12::ShaderVisibleDescriptorHeapDX12(uint aNumBindlessSamplers, uint aNumTransientDescriptors, uint aNumTransientDescriptorsPerRange)
    : myOverallNumBindlessDescriptors(aNumBindlessSamplers)
    , myNumBindlessDescriptors{0, 0, 0, 0, aNumBindlessSamplers}
    , myNumTransientDescriptors(aNumTransientDescriptors)
    , myNumTransientDescriptorsPerRange(aNumTransientDescriptorsPerRange)
    , myNumTransientRanges(aNumTransientDescriptors / aNumTransientDescriptorsPerRange)
    , myBindlessAllocators{
        PagedLinearAllocator(0),
        PagedLinearAllocator(0),
        PagedLinearAllocator(0),
        PagedLinearAllocator(0),
        PagedLinearAllocator(aNumBindlessSamplers)
    }
  {
    ASSERT(aNumTransientDescriptorsPerRange <= aNumTransientDescriptors);

    Init(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
  }
//---------------------------------------------------------------------------//
  void ShaderVisibleDescriptorHeapDX12::Init(D3D12_DESCRIPTOR_HEAP_TYPE aType)
  {
    myNextFreeTransientDescriptorIdx = myOverallNumBindlessDescriptors;

    myTransientRangeLastUseFences.resize(myNumTransientRanges, 0ull);

    const uint numDescriptors = myOverallNumBindlessDescriptors + myNumTransientDescriptors;
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    heapDesc.NumDescriptors = numDescriptors;
    heapDesc.Type = aType;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.NodeMask = 0u;

    ID3D12Device* device = RenderCore::GetPlatformDX12()->GetDevice();

    ASSERT_HRESULT(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&myDescriptorHeap)));
    myDesc = heapDesc;

    myHandleIncrementSize = device->GetDescriptorHandleIncrementSize(aType);
    myCpuHeapStart = myDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    myGpuHeapStart = myDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

    uint offset = 0;
    for (uint i = 0; i < BINDLESS_DESCRIPTOR_NUM; ++i)
    {
      ASSERT(myBindlessAllocators[i].IsEmpty());
      
      myBindlessDescriptorCpuHeapStart[i] = myCpuHeapStart;
      myBindlessDescriptorCpuHeapStart[i].ptr += offset * myHandleIncrementSize;

      myBindlessDescriptorGpuHeapStart[i] = myGpuHeapStart;
      myBindlessDescriptorGpuHeapStart[i].ptr += offset * myHandleIncrementSize;

      if (myNumBindlessDescriptors[i] > 0)
      {
        // Preinitialize heap with appropriate null-descriptors

        const BindlessDescriptorType bindlessType = static_cast<BindlessDescriptorType>(i);

        const DescriptorDX12& nullDescriptor = 
          RenderCore::GetPlatformDX12()->GetNullDescriptor(Private::GetDescriptorRangeType(bindlessType), Private::GetResourceDimension(bindlessType));

        for (uint k = 0; k < myNumBindlessDescriptors[i]; ++k)
        {
          D3D12_CPU_DESCRIPTOR_HANDLE dstDescriptor = myBindlessDescriptorCpuHeapStart[i];
          dstDescriptor.ptr += k * myHandleIncrementSize;

          device->CopyDescriptorsSimple(1, dstDescriptor, nullDescriptor.myCpuHandle, aType);
        }
        
        //uint dstRangeSize = myNumBindlessDescriptors[i];
        //uint srcRangeSize = 1;
        //
        //device->CopyDescriptors(1, &myBindlessDescriptorCpuHeapStart[i], &dstRangeSize,
        //  1, &nullDescriptor.myCpuHandle, &srcRangeSize, aType);
      }

      offset += myNumBindlessDescriptors[i];
    }
  }
//---------------------------------------------------------------------------//
  void ShaderVisibleDescriptorHeapDX12::ProcessBindlessDescriptorFrees()
  {
    if (Time::ourFrameIdx < ourNumBindlessFreeLists - 1)
      return;

    const uint64 frameIdxToProcess = Time::ourFrameIdx - static_cast<uint64>(ourNumBindlessFreeLists) - 1u;
    ASSERT(RenderCore::IsFrameDone(frameIdxToProcess));

    const uint freeListToProcess = frameIdxToProcess % ourNumBindlessFreeLists;
    for (uint i = 0; i < BINDLESS_DESCRIPTOR_NUM; ++i)
    {
      const BindlessDescriptorType bindlessType = static_cast<BindlessDescriptorType>(i);
      const DescriptorDX12& nullDescriptor =
        RenderCore::GetPlatformDX12()->GetNullDescriptor(Private::GetDescriptorRangeType(bindlessType), Private::GetResourceDimension(bindlessType));

      for (const uint descriptorIdxToFree : myBindlessDescriptorsToFree[freeListToProcess][i])
      {
        myBindlessAllocators[i].Free({descriptorIdxToFree, descriptorIdxToFree + 1});

#if FANCY_HEAVY_DEBUG
        // Replace with null descriptor
        // For debugging, this could also become a special error-resource for detecting if something deleted is accessed in a shader.
        D3D12_CPU_DESCRIPTOR_HANDLE dst;
        dst.ptr = myBindlessDescriptorCpuHeapStart[i].ptr + descriptorIdxToFree * myHandleIncrementSize;
        RenderCore::GetPlatformDX12()->GetDevice()->CopyDescriptorsSimple(1, dst, nullDescriptor.myCpuHandle, myDesc.Type);
#endif
      }

      myBindlessDescriptorsToFree[freeListToProcess][i].clear();
    }
  }
//---------------------------------------------------------------------------//
  BindlessDescriptorType ShaderVisibleDescriptorHeapDX12::GetBindlessType(const DescriptorDX12& aDescriptor)
  {
    ASSERT(aDescriptor.myHeapType == myDesc.Type);
    ASSERT(aDescriptor.myIsManagedByAllocator && aDescriptor.myIsShaderVisible);

    for (uint i = 0; i < BINDLESS_DESCRIPTOR_NUM; ++i)
    {
      if (aDescriptor.myCpuHandle.ptr >= myBindlessDescriptorCpuHeapStart[i].ptr)
      {
        const uint64 endPtr = i < BINDLESS_DESCRIPTOR_NUM - 1 ? myBindlessDescriptorCpuHeapStart[i + 1].ptr : myBindlessDescriptorCpuHeapStart[i].ptr + myNumBindlessDescriptors[i] * myHandleIncrementSize;
        if (aDescriptor.myCpuHandle.ptr < endPtr)
          return static_cast<BindlessDescriptorType>(i);
      }
    }

    ASSERT(false);
    return BINDLESS_DESCRIPTOR_NUM;
  }
//---------------------------------------------------------------------------//
  ShaderVisibleDescriptorHeapDX12::RangeAllocation ShaderVisibleDescriptorHeapDX12::AllocateTransientRange()
  {
    std::lock_guard<std::mutex> lock(myRangeAllocMutex);

    if (GetNumFreeTransientDescriptors() < myNumTransientDescriptorsPerRange)
    {
      // Wrap around to start
      myNextFreeTransientDescriptorIdx = myOverallNumBindlessDescriptors;
    }

    const uint rangeIdx = (myNextFreeTransientDescriptorIdx - myOverallNumBindlessDescriptors) / myNumTransientDescriptorsPerRange;
    const uint64 rangeLastUseFence = myTransientRangeLastUseFences[rangeIdx];
    ASSERT(RenderCore::IsFenceDone(rangeLastUseFence), 
      "Trying to allocate a dynamic descriptor range that hasn't been fully processed by the GPU yet. Consider increasing the size of the dynamic (shader visible) descriptor heap");

    RangeAllocation alloc;
    alloc.myHeap = this;
    alloc.myFirstDescriptorIndexInHeap = myNextFreeTransientDescriptorIdx;
    alloc.myNumDescriptors = myNumTransientDescriptorsPerRange;
    alloc.myNumAllocatedDescriptors = 0u;

    myNextFreeTransientDescriptorIdx += myNumTransientDescriptorsPerRange;

    return alloc;
  }
//---------------------------------------------------------------------------//
  void ShaderVisibleDescriptorHeapDX12::FreeTransientRange(const RangeAllocation& aRange, uint64 aFence)
  {
    ASSERT(aRange.myHeap == this, "Trying to free a RangeAllocation that doesn't belong to this heap");

    const uint rangeIdx = (aRange.myFirstDescriptorIndexInHeap - myOverallNumBindlessDescriptors) / myNumTransientDescriptorsPerRange;
    myTransientRangeLastUseFences[rangeIdx] = glm::max(myTransientRangeLastUseFences[rangeIdx], aFence);
  }
//---------------------------------------------------------------------------//
  DescriptorDX12 ShaderVisibleDescriptorHeapDX12::AllocateDescriptor(BindlessDescriptorType aType)
  {
    ASSERT(myBindlessAllocators[aType].GetPageSize() != 0);

    uint64 offset;
    const PagedLinearAllocator::Page* page = myBindlessAllocators[aType].Allocate(1, 0, offset);
    ASSERT(page, "Failed allocating shader visible descriptor. Consider increasing the max bindless descriptor sizes");

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    cpuHandle.ptr = myBindlessDescriptorCpuHeapStart[aType].ptr + offset * myHandleIncrementSize;

    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    gpuHandle.ptr = myBindlessDescriptorGpuHeapStart[aType].ptr + offset * myHandleIncrementSize;

    return { cpuHandle, gpuHandle, myDesc.Type, true, true };
  }
//---------------------------------------------------------------------------//
  void ShaderVisibleDescriptorHeapDX12::FreeDescriptorAfterFrame(const DescriptorDX12& aDescriptor)
  {
    const BindlessDescriptorType type = GetBindlessType(aDescriptor);
    ASSERT(myBindlessAllocators[type].GetPageSize() != 0);

    const uint offset = (aDescriptor.myCpuHandle.ptr - myBindlessDescriptorCpuHeapStart[type].ptr) / myHandleIncrementSize;
    const uint freeListIdx = Time::ourFrameIdx % ourNumBindlessFreeLists;
    myBindlessDescriptorsToFree[freeListIdx][type].push_back(offset);
  }
//---------------------------------------------------------------------------//
  D3D12_GPU_DESCRIPTOR_HANDLE ShaderVisibleDescriptorHeapDX12::GetBindlessHeapStart(BindlessDescriptorType aType) const
  {
    return myBindlessDescriptorGpuHeapStart[aType];
  }
//---------------------------------------------------------------------------//
  DescriptorDX12 ShaderVisibleDescriptorHeapDX12::GetDescriptor(uint anIndex) const
  {
    ASSERT(anIndex < myDesc.NumDescriptors);

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    cpuHandle.ptr = myCpuHeapStart.ptr + anIndex * myHandleIncrementSize;

    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    gpuHandle.ptr = myGpuHeapStart.ptr + anIndex * myHandleIncrementSize;

    DescriptorDX12 descr;
    descr.myCpuHandle = cpuHandle;
    descr.myGpuHandle = gpuHandle;
    descr.myHeapType = myDesc.Type;

    return descr;
  }
//---------------------------------------------------------------------------//
}


#endif