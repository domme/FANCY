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
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.NumDescriptors = kMaxNumDescriptorsPerCpuHeap;
    heapDesc.NodeMask = 0;

    for (uint32 i = 0u; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
    {
      heapDesc.Type = static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i);
      myCpuHeaps[i].Create(aRenderer.GetDevice(), heapDesc);
    }
  }
//---------------------------------------------------------------------------//
  DescriptorHeapPoolDX12::~DescriptorHeapPoolDX12()
  {
    while (!myUsedShaderVisibleHeaps.empty())
    {
      myRenderer.WaitForFence(myUsedShaderVisibleHeaps.front().first);
      myUsedShaderVisibleHeaps.pop();
    }

    myAvailableShaderVisibleHeaps.clear();
  }
//---------------------------------------------------------------------------//
  DescriptorHeapDX12* DescriptorHeapPoolDX12::AllocateShaderVisibleHeap(uint32 aRequiredNumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE aType)
  {
    while(!myUsedShaderVisibleHeaps.empty() && myRenderer.IsFenceDone(myUsedShaderVisibleHeaps.front().first))
    {
      myAvailableShaderVisibleHeaps.push_back(myUsedShaderVisibleHeaps.front().second);
      myUsedShaderVisibleHeaps.pop();
    }

    if (aRequiredNumDescriptors % kGpuDescriptorNumIncrement > 0)
      aRequiredNumDescriptors = aRequiredNumDescriptors + (kGpuDescriptorNumIncrement - aRequiredNumDescriptors % kGpuDescriptorNumIncrement);

    for (auto it = myAvailableShaderVisibleHeaps.begin(); it != myAvailableShaderVisibleHeaps.end(); ++it)
    {
      DescriptorHeapDX12* heap = (*it);
      if (heap->myDesc.NumDescriptors == aRequiredNumDescriptors && heap->myDesc.Type == aType)
      {
        myAvailableShaderVisibleHeaps.erase(it);
        return heap;
      }
    }

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    heapDesc.NumDescriptors = aRequiredNumDescriptors;
    heapDesc.Type = aType;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    myShaderVisibleHeapPool.push_back(std::make_unique<DescriptorHeapDX12>(myRenderer.GetDevice(), heapDesc));
    return myShaderVisibleHeapPool.back().get();
  }
//---------------------------------------------------------------------------//
  void DescriptorHeapPoolDX12::ReleaseShaderVisibleHeap(uint64 aFenceVal, DescriptorHeapDX12* aUsedHeap)
  {
    myUsedShaderVisibleHeaps.push(std::make_pair(aFenceVal, aUsedHeap));
  }
//---------------------------------------------------------------------------//
} } }