#include "FancyCorePrerequisites.h"
#include  "DescriptorHeapPoolDX12.h"

#include "RendererDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  DescriptorHeapDX12::DescriptorHeapDX12(ID3D12Device* aDevice, const D3D12_DESCRIPTOR_HEAP_DESC& aDesc)
    : myNextFreeHandleIndex(0u)
    , myHandleIncrementSize(0u)
  {
    Create(aDevice, aDesc);
  }
//---------------------------------------------------------------------------//
  Descriptor DescriptorHeapDX12::AllocateDescriptor()
  {
    ASSERT(myNextFreeHandleIndex < myDesc.NumDescriptors);

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    cpuHandle.ptr = myCpuHeapStart.ptr + myNextFreeHandleIndex * myHandleIncrementSize;

    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    gpuHandle.ptr = myGpuHeapStart.ptr + myNextFreeHandleIndex * myHandleIncrementSize;

    ++myNextFreeHandleIndex;

    Descriptor descr;
    descr.myCpuHandle = cpuHandle;
    descr.myGpuHandle = gpuHandle;

    return descr;
  }
//---------------------------------------------------------------------------//
  Descriptor DescriptorHeapDX12::GetDescriptor(uint32 anIndex)
  {
    ASSERT(anIndex < myNextFreeHandleIndex);

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    cpuHandle.ptr = myCpuHeapStart.ptr + anIndex * myHandleIncrementSize;

    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    gpuHandle.ptr = myGpuHeapStart.ptr + anIndex * myHandleIncrementSize;

    Descriptor descr;
    descr.myCpuHandle = cpuHandle;
    descr.myGpuHandle = gpuHandle;

    return descr;
  }
//---------------------------------------------------------------------------//
  DescriptorHeapDX12::DescriptorHeapDX12()
    : myHandleIncrementSize(0u)
    , myNextFreeHandleIndex(0u)
  {

  }
//---------------------------------------------------------------------------//
  void DescriptorHeapDX12::Create(ID3D12Device* aDevice, const D3D12_DESCRIPTOR_HEAP_DESC& aDesc)
  {
    CheckD3Dcall(aDevice->CreateDescriptorHeap(&aDesc, IID_PPV_ARGS(&myDescriptorHeap)));
    myDesc = aDesc;

    myHandleIncrementSize = aDevice->GetDescriptorHandleIncrementSize(aDesc.Type);
    myCpuHeapStart = myDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    myGpuHeapStart = myDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
  }
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
  DescriptorHeapPoolDX12::DescriptorHeapPoolDX12(RendererDX12& aRenderer)
    : myRenderer(aRenderer)
  {
    const uint32 kMaxNumStaticDescriptorsPerHeap = 1000u;

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    heapDesc.NumDescriptors = kMaxNumStaticDescriptorsPerHeap;
    heapDesc.NodeMask = 0u;
    
    for (uint32 i = 0u; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
    {
      //if(i == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || i == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
      //  heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
      //else
        heapDesc.Flags =  D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

      heapDesc.Type = static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i);
      myStaticHeaps[i].Create(aRenderer.GetDevice(), heapDesc);
    }
  }
//---------------------------------------------------------------------------//
  DescriptorHeapPoolDX12::~DescriptorHeapPoolDX12()
  {
    while (!myUsedDynamicHeaps.empty())
    {
      myRenderer.WaitForFence(myUsedDynamicHeaps.front().first);
      myUsedDynamicHeaps.pop();
    }

    myAvailableDynamicHeaps.clear();
  }
//---------------------------------------------------------------------------//
  DescriptorHeapDX12* DescriptorHeapPoolDX12::AllocateDynamicHeap(uint32 aRequiredNumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE aType)
  {
    while(!myUsedDynamicHeaps.empty() && myRenderer.IsFenceDone(myUsedDynamicHeaps.front().first))
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
    myDynamicHeapPool.push_back(std::make_unique<DescriptorHeapDX12>(myRenderer.GetDevice(), heapDesc));
    return myDynamicHeapPool.back().get();
  }
//---------------------------------------------------------------------------//
  void DescriptorHeapPoolDX12::ReleaseDynamicHeap(uint64 aFenceVal, DescriptorHeapDX12* aUsedHeap)
  {
    myUsedDynamicHeaps.push(std::make_pair(aFenceVal, aUsedHeap));
  }
//---------------------------------------------------------------------------//
} } }