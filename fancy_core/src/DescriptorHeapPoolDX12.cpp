#include "FancyCorePrerequisites.h"
#include "DescriptorHeapPoolDX12.h"

#include "MathUtil.h"
#include "DescriptorDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  DescriptorHeapDX12::DescriptorHeapDX12(const D3D12_DESCRIPTOR_HEAP_DESC& aDesc)
    : myNextFreeHandleIndex(0u)
    , myHandleIncrementSize(0u)
  {
    Create(aDesc);
  }
//---------------------------------------------------------------------------//
  DescriptorDX12 DescriptorHeapDX12::AllocateDescriptor()
  {
    ASSERT(myNextFreeHandleIndex < myDesc.NumDescriptors);

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    cpuHandle.ptr = myCpuHeapStart.ptr + myNextFreeHandleIndex * myHandleIncrementSize;

    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    gpuHandle.ptr = myGpuHeapStart.ptr + myNextFreeHandleIndex * myHandleIncrementSize;

    ++myNextFreeHandleIndex;

    DescriptorDX12 descr;
    descr.myCpuHandle = cpuHandle;
    descr.myGpuHandle = gpuHandle;
    descr.myHeapType = myDesc.Type;

    return descr;
  }
//---------------------------------------------------------------------------//
  DescriptorDX12 DescriptorHeapDX12::GetDescriptor(uint32 anIndex) const
  {
    ASSERT(anIndex < myNextFreeHandleIndex);

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
  DescriptorHeapDX12::DescriptorHeapDX12()
    : myHandleIncrementSize(0u)
    , myNextFreeHandleIndex(0u)
  {

  }
//---------------------------------------------------------------------------//
  void DescriptorHeapDX12::Create(const D3D12_DESCRIPTOR_HEAP_DESC& aDesc)
  {
    CheckD3Dcall(RenderCore::GetPlatformDX12()->GetDevice()->CreateDescriptorHeap(&aDesc, IID_PPV_ARGS(&myDescriptorHeap)));
    myDesc = aDesc;

    myHandleIncrementSize = 
      RenderCore::GetPlatformDX12()->GetDevice()->GetDescriptorHandleIncrementSize(aDesc.Type);
    myCpuHeapStart = myDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    myGpuHeapStart = myDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
  }
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
  DescriptorHeapPoolDX12::DescriptorHeapPoolDX12()
  {
    
  }
//---------------------------------------------------------------------------//
  DescriptorHeapPoolDX12::~DescriptorHeapPoolDX12()
  {
    while (!myUsedDynamicHeaps.empty())
    {
      RenderCore::GetPlatformDX12()->WaitForFence(myUsedDynamicHeaps.front().first.myType, 
                               myUsedDynamicHeaps.front().first.myFenceVal);
      myUsedDynamicHeaps.pop();
    }

    myAvailableDynamicHeaps.clear();
  }
//---------------------------------------------------------------------------//
  DescriptorHeapDX12* DescriptorHeapPoolDX12::AllocateDynamicHeap(uint32 aRequiredNumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE aType)
  {
    while(!myUsedDynamicHeaps.empty() 
      && RenderCore::GetPlatformDX12()->IsFenceDone(myUsedDynamicHeaps.front().first.myType,
                                 myUsedDynamicHeaps.front().first.myFenceVal))
    {
      DescriptorHeapDX12* heap = myUsedDynamicHeaps.front().second;
      heap->Reset();

      myAvailableDynamicHeaps.push_back(heap);
      myUsedDynamicHeaps.pop();
    }

    aRequiredNumDescriptors = MathUtil::Align(aRequiredNumDescriptors, kGpuDescriptorNumIncrement);

    for (auto it = myAvailableDynamicHeaps.begin(); it != myAvailableDynamicHeaps.end(); ++it)
    {
      DescriptorHeapDX12* heap = (*it);
      if (heap->myDesc.NumDescriptors == aRequiredNumDescriptors && heap->myDesc.Type == aType)
      {
        myAvailableDynamicHeaps.erase(it);
        return heap;
      }
    }

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    heapDesc.NumDescriptors = aRequiredNumDescriptors;
    heapDesc.Type = aType;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.NodeMask = 0u;
    myDynamicHeapPool.push_back(std::make_unique<DescriptorHeapDX12>(heapDesc));
    return myDynamicHeapPool.back().get();
  }
//---------------------------------------------------------------------------//
  void DescriptorHeapPoolDX12::ReleaseDynamicHeap(CommandListType aCmdListType, uint64 aFenceVal, DescriptorHeapDX12* aUsedHeap)
  {
    FenceInfo info;
    info.myType = aCmdListType;
    info.myFenceVal = aFenceVal;

    myUsedDynamicHeaps.push(std::make_pair(info, aUsedHeap));
  }
//---------------------------------------------------------------------------//
} } }
