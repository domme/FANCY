#include "FancyCorePrerequisites.h"
#include "DynamicDescriptorHeapDX12.h"

#include "DescriptorDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  DynamicDescriptorHeapDX12::DynamicDescriptorHeapDX12(D3D12_DESCRIPTOR_HEAP_TYPE aType, uint64 aNumDescriptors)
    : myNextFreeHandleIndex(0u)
    , myHandleIncrementSize(0u)
  {
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    heapDesc.NumDescriptors = aNumDescriptors;
    heapDesc.Type = aType;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.NodeMask = 0u;

    ID3D12Device* device = RenderCore::GetPlatformDX12()->GetDevice();

    CheckD3Dcall(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&myDescriptorHeap)));
    myDesc = heapDesc;

    myHandleIncrementSize = device->GetDescriptorHandleIncrementSize(aType);
    myCpuHeapStart = myDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    myGpuHeapStart = myDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
  }
//---------------------------------------------------------------------------//
  DescriptorDX12 DynamicDescriptorHeapDX12::AllocateDescriptor()
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
  DescriptorDX12 DynamicDescriptorHeapDX12::GetDescriptor(uint anIndex) const
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
  DynamicDescriptorHeapDX12::DynamicDescriptorHeapDX12()
    : myHandleIncrementSize(0u)
    , myNextFreeHandleIndex(0u)
  {

  }
//---------------------------------------------------------------------------//
}