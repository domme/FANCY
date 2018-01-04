#include "FancyCorePrerequisites.h"
#include "DescriptorHeapDX12.h"

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
} } }