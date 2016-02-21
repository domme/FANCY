#include "FancyCorePrerequisites.h"
#include  "DescriptorHeapPoolDX12.h"

#include "RendererDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  DescriptorHeapDX12::DescriptorHeapDX12(ID3D12Device* aDevice, const D3D12_DESCRIPTOR_HEAP_DESC& aDesc)
  {
    Create(aDevice, aDesc);
  }
//---------------------------------------------------------------------------//
  DescriptorHeapDX12::DescriptorHeapDX12()
    : myHandleIncrementSize(0u)
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
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    heapDesc.NumDescriptors = kMaxNumDescriptorsPerCpuHeap;

    for (uint32 i = 0u; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
    {
      heapDesc.Type = static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i);
      myCpuVisibleHeaps[i].Create(aRenderer.GetDevice(), heapDesc);
    }
  }
//---------------------------------------------------------------------------//
  DescriptorHeapPoolDX12::~DescriptorHeapPoolDX12()
  {
    while (!myUsedGpuVisibleHeaps.empty())
    {
      myRenderer.WaitForFence(myUsedGpuVisibleHeaps.front().first);
      myUsedGpuVisibleHeaps.pop();
    }

    myAvailableGpuVisibleHeaps.clear();
  }
//---------------------------------------------------------------------------//
  DescriptorHeapDX12* DescriptorHeapPoolDX12::GetGpuHeap(uint32 aRequiredNumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE aType)
  {
    while(!myUsedGpuVisibleHeaps.empty() && myRenderer.IsFenceDone(myUsedGpuVisibleHeaps.front().first))
    {
      myAvailableGpuVisibleHeaps.push_back(myUsedGpuVisibleHeaps.front().second);
      myUsedGpuVisibleHeaps.pop();
    }

    if (aRequiredNumDescriptors % kGpuDescriptorNumIncrement > 0)
      aRequiredNumDescriptors = aRequiredNumDescriptors + (kGpuDescriptorNumIncrement - aRequiredNumDescriptors % kGpuDescriptorNumIncrement);

    for (auto it = myAvailableGpuVisibleHeaps.begin(); it != myAvailableGpuVisibleHeaps.end(); ++it)
    {
      DescriptorHeapDX12* heap = (*it);
      if (heap->myDesc.NumDescriptors == aRequiredNumDescriptors && heap->myDesc.Type == aType)
      {
        myAvailableGpuVisibleHeaps.erase(it);
        return heap;
      }
    }

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    heapDesc.NumDescriptors = aRequiredNumDescriptors;
    heapDesc.Type = aType;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    myDescriptorHeapPool.push_back(std::make_unique<DescriptorHeapDX12>(myRenderer.GetDevice(), heapDesc));
    return myDescriptorHeapPool.back().get();
  }
//---------------------------------------------------------------------------//
  void DescriptorHeapPoolDX12::ReleaseGpuHeap(uint64 aFenceVal, DescriptorHeapDX12* aUsedHeap)
  {
    myUsedGpuVisibleHeaps.push(std::make_pair(aFenceVal, aUsedHeap));
  }
//---------------------------------------------------------------------------//
} } }