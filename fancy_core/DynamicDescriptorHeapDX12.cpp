#include "fancy_core_precompile.h"
#include "DynamicDescriptorHeapDX12.h"

#include "DescriptorDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
//---------------------------------------------------------------------------//
  namespace Private
  {
    static D3D12_DESCRIPTOR_RANGE_TYPE GetDescriptorRangeType(DynamicDescriptorHeapDX12::BindlessDescriptorType aBindlessType)
    {
      static D3D12_DESCRIPTOR_RANGE_TYPE rangeType[] =
      {
        D3D12_DESCRIPTOR_RANGE_TYPE_SRV,    // BINDLESS_TEXTURE_2D
        D3D12_DESCRIPTOR_RANGE_TYPE_UAV,    // BINLDESS_RW_TEXTURE
        D3D12_DESCRIPTOR_RANGE_TYPE_SRV,    // BINDLESS_BUFFER
        D3D12_DESCRIPTOR_RANGE_TYPE_UAV,    // BINDLESS_RW_BUFFER
        D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER // BINDLESS_SAMPLER
      };

      return rangeType[aBindlessType];
    }
  }
//---------------------------------------------------------------------------//
  DynamicDescriptorHeapDX12::DynamicDescriptorHeapDX12(uint aNumBindlessTextures, uint aNumBindlessRWTextures, uint aNumBindlessBuffers, uint aNumBindlessRWBuffers,
    uint aNumTransientDescriptors, uint aNumTransientDescriptorsPerRange)
    : myOverallNumBindlessDescriptors(aNumBindlessTextures + aNumBindlessBuffers + aNumBindlessRWTextures + aNumBindlessRWBuffers)
    , myNumBindlessDescriptors{ aNumBindlessTextures, aNumBindlessRWTextures, aNumBindlessBuffers, aNumBindlessRWBuffers, 0 }
    , myNumTransientDescriptors(aNumTransientDescriptors)
    , myNumTransientDescriptorsPerRange(aNumTransientDescriptorsPerRange)
    , myNextFreeBindlessDescriptorIdx{}
    , myNumTransientRanges(aNumTransientDescriptors / aNumTransientDescriptorsPerRange)
  {
    ASSERT(aNumTransientDescriptorsPerRange <= aNumTransientDescriptors);

    Init(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  }
//---------------------------------------------------------------------------//
  DynamicDescriptorHeapDX12::DynamicDescriptorHeapDX12(uint aNumBindlessSamplers, uint aNumTransientDescriptors, uint aNumTransientDescriptorsPerRange)
    : myOverallNumBindlessDescriptors(aNumBindlessSamplers)
    , myNumBindlessDescriptors{0, 0, 0, 0, aNumBindlessSamplers}
    , myNumTransientDescriptors(aNumTransientDescriptors)
    , myNumTransientDescriptorsPerRange(aNumTransientDescriptorsPerRange)
    , myNextFreeBindlessDescriptorIdx{}
    , myNumTransientRanges(aNumTransientDescriptors / aNumTransientDescriptorsPerRange)
  {
    ASSERT(aNumTransientDescriptorsPerRange <= aNumTransientDescriptors);

    Init(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
  }
//---------------------------------------------------------------------------//
  void DynamicDescriptorHeapDX12::Init(D3D12_DESCRIPTOR_HEAP_TYPE aType)
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
    for (uint i = 0; i < BINDLESS_NUM; ++i)
    {
      myNextFreeBindlessDescriptorIdx[i] = offset;
      
      myBindlessDescriptorCpuHeapStart[i] = myCpuHeapStart;
      myBindlessDescriptorCpuHeapStart[i].ptr += offset * myHandleIncrementSize;

      myBindlessDescriptorGpuHeapStart[i] = myGpuHeapStart;
      myBindlessDescriptorGpuHeapStart[i].ptr += offset * myHandleIncrementSize;

      if (myNumBindlessDescriptors[i] > 0)
      {
        // Preinitialize heap with appropriate null-descriptors
        GpuResourceDimension resourceDimension = GpuResourceDimension::UNKONWN;
        switch((BindlessDescriptorType) i)
        {
        case BINDLESS_TEXTURE_2D:
        case BINDLESS_RW_TEXTURE_2D:
          resourceDimension = GpuResourceDimension::TEXTURE_2D;
          break;
        case BINDLESS_BUFFER:
        case BINDLESS_RW_BUFFER:
          resourceDimension = GpuResourceDimension::BUFFER;
          break;
        case BINDLESS_SAMPLER:
          resourceDimension = GpuResourceDimension::UNKONWN;
          break;
        default: ASSERT(false);
        }

        const DescriptorDX12& nullDescriptor = 
          RenderCore::GetPlatformDX12()->GetNullDescriptor(Private::GetDescriptorRangeType(static_cast<BindlessDescriptorType>(i)), resourceDimension);

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
  DynamicDescriptorHeapDX12::RangeAllocation DynamicDescriptorHeapDX12::AllocateTransientRange()
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
  void DynamicDescriptorHeapDX12::FreeTransientRange(const RangeAllocation& aRange, uint64 aFence)
  {
    ASSERT(aRange.myHeap == this, "Trying to free a RangeAllocation that doesn't belong to this heap");

    const uint rangeIdx = (aRange.myFirstDescriptorIndexInHeap - myOverallNumBindlessDescriptors) / myNumTransientDescriptorsPerRange;
    myTransientRangeLastUseFences[rangeIdx] = glm::max(myTransientRangeLastUseFences[rangeIdx], aFence);
  }

  D3D12_GPU_DESCRIPTOR_HANDLE DynamicDescriptorHeapDX12::GetBindlessHeapStart(BindlessDescriptorType aType) const
  {
    return myBindlessDescriptorGpuHeapStart[aType];
  }

//---------------------------------------------------------------------------//
  DescriptorDX12 DynamicDescriptorHeapDX12::AllocateConstantDescriptorRange(uint /*aNumDescriptors*/)
  {
    ASSERT(false, "Deprecated!");
    return DescriptorDX12();

    //const DescriptorDX12 rangeStartDescriptor = GetDescriptor(myNextFreeBindlessDescriptorIdx);
    //myNextFreeBindlessDescriptorIdx += aNumDescriptors;
    //
    //return rangeStartDescriptor;
  }
//---------------------------------------------------------------------------//
  DescriptorDX12 DynamicDescriptorHeapDX12::GetDescriptor(uint anIndex) const
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