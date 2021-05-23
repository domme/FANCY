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
    static D3D12_DESCRIPTOR_RANGE_TYPE GetDescriptorRangeType(GlobalResourceType aGlobalType)
    {
      switch(aGlobalType)
      {
      case GLOBAL_RESOURCE_TEXTURE_2D: return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
      case GLOBAL_RESOURCE_RW_TEXTURE_2D: return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
      case GLOBAL_RESOURCE_BUFFER: return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
      case GLOBAL_RESOURCE_RW_BUFFER: return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
      case GLOBAL_RESOURCE_SAMPLER: return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
      case GLOBAL_RESOURCE_NUM: ASSERT(false); return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
      }
    }

    static GpuResourceDimension GetResourceDimension(GlobalResourceType aGlobalType)
    {
      GpuResourceDimension resourceDimension = GpuResourceDimension::UNKONWN;
      switch (aGlobalType)
      {
      case GLOBAL_RESOURCE_TEXTURE_2D:
      case GLOBAL_RESOURCE_RW_TEXTURE_2D:
        return GpuResourceDimension::TEXTURE_2D;
      case GLOBAL_RESOURCE_BUFFER:
      case GLOBAL_RESOURCE_RW_BUFFER:
        return GpuResourceDimension::BUFFER;
      case GLOBAL_RESOURCE_SAMPLER:
        return GpuResourceDimension::UNKONWN;
      default: ASSERT(false); return GpuResourceDimension::TEXTURE_2D;
      }
    }
  }
//---------------------------------------------------------------------------//
  ShaderVisibleDescriptorHeapDX12::ShaderVisibleDescriptorHeapDX12(uint aNumGlobalTextures, uint aNumGlobalRWTextures, uint aNumGlobalBuffers, uint aNumGlobalRWBuffers)
    : myOverallNumGlobalDescriptors(aNumGlobalTextures + aNumGlobalBuffers + aNumGlobalRWTextures + aNumGlobalRWBuffers)
    , myNumGlobalDescriptors{ aNumGlobalTextures, aNumGlobalRWTextures, aNumGlobalBuffers, aNumGlobalRWBuffers, 0 }
    , myGlobalAllocators{
      PagedLinearAllocator(aNumGlobalTextures),
      PagedLinearAllocator(aNumGlobalRWTextures),
      PagedLinearAllocator(aNumGlobalBuffers),
      PagedLinearAllocator(aNumGlobalRWBuffers),
      PagedLinearAllocator(0)
    }
  {
    Init(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  }
//---------------------------------------------------------------------------//
  ShaderVisibleDescriptorHeapDX12::ShaderVisibleDescriptorHeapDX12(uint aNumGlobalSamplers)
    : myOverallNumGlobalDescriptors(aNumGlobalSamplers)
    , myNumGlobalDescriptors{0, 0, 0, 0, aNumGlobalSamplers}
    , myGlobalAllocators{
        PagedLinearAllocator(0),
        PagedLinearAllocator(0),
        PagedLinearAllocator(0),
        PagedLinearAllocator(0),
        PagedLinearAllocator(aNumGlobalSamplers)
    }
  {
    Init(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
  }
//---------------------------------------------------------------------------//
  void ShaderVisibleDescriptorHeapDX12::Init(D3D12_DESCRIPTOR_HEAP_TYPE aType)
  {
    const uint numDescriptors = myOverallNumGlobalDescriptors;
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
    for (uint i = 0; i < GLOBAL_RESOURCE_NUM; ++i)
    {
      ASSERT(myGlobalAllocators[i].IsEmpty());
      
      myGlobalDescriptorCpuHeapStart[i] = myCpuHeapStart;
      myGlobalDescriptorCpuHeapStart[i].ptr += offset * myHandleIncrementSize;

      myGlobalDescriptorGpuHeapStart[i] = myGpuHeapStart;
      myGlobalDescriptorGpuHeapStart[i].ptr += offset * myHandleIncrementSize;

      if (myNumGlobalDescriptors[i] > 0)
      {
        // Preinitialize heap with appropriate null-descriptors

        const GlobalResourceType GlobalType = static_cast<GlobalResourceType>(i);

        const DescriptorDX12& nullDescriptor = 
          RenderCore::GetPlatformDX12()->GetNullDescriptor(Private::GetDescriptorRangeType(GlobalType), Private::GetResourceDimension(GlobalType));

        for (uint k = 0; k < myNumGlobalDescriptors[i]; ++k)
        {
          D3D12_CPU_DESCRIPTOR_HANDLE dstDescriptor = myGlobalDescriptorCpuHeapStart[i];
          dstDescriptor.ptr += k * myHandleIncrementSize;

          device->CopyDescriptorsSimple(1, dstDescriptor, nullDescriptor.myCpuHandle, aType);
        }
        
        //uint dstRangeSize = myNumGlobalDescriptors[i];
        //uint srcRangeSize = 1;
        //
        //device->CopyDescriptors(1, &myGlobalDescriptorCpuHeapStart[i], &dstRangeSize,
        //  1, &nullDescriptor.myCpuHandle, &srcRangeSize, aType);
      }

      offset += myNumGlobalDescriptors[i];
    }
  }
//---------------------------------------------------------------------------//
  void ShaderVisibleDescriptorHeapDX12::ProcessGlobalDescriptorFrees()
  {
    if (Time::ourFrameIdx < ourNumGlobalFreeLists - 1)
      return;

    const uint64 frameIdxToProcess = Time::ourFrameIdx - static_cast<uint64>(ourNumGlobalFreeLists) - 1u;
    ASSERT(RenderCore::IsFrameDone(frameIdxToProcess));

    const uint freeListToProcess = frameIdxToProcess % ourNumGlobalFreeLists;
    for (uint i = 0; i < GLOBAL_RESOURCE_NUM; ++i)
    {
      const GlobalResourceType GlobalType = static_cast<GlobalResourceType>(i);
      const DescriptorDX12& nullDescriptor =
        RenderCore::GetPlatformDX12()->GetNullDescriptor(Private::GetDescriptorRangeType(GlobalType), Private::GetResourceDimension(GlobalType));

      for (const uint descriptorIdxToFree : myGlobalDescriptorsToFree[freeListToProcess][i])
      {
        myGlobalAllocators[i].Free({descriptorIdxToFree, descriptorIdxToFree + 1});

#if FANCY_HEAVY_DEBUG
        // Replace with null descriptor
        // For debugging, this could also become a special error-resource for detecting if something deleted is accessed in a shader.
        D3D12_CPU_DESCRIPTOR_HANDLE dst;
        dst.ptr = myGlobalDescriptorCpuHeapStart[i].ptr + descriptorIdxToFree * myHandleIncrementSize;
        RenderCore::GetPlatformDX12()->GetDevice()->CopyDescriptorsSimple(1, dst, nullDescriptor.myCpuHandle, myDesc.Type);
#endif
      }

      myGlobalDescriptorsToFree[freeListToProcess][i].clear();
    }
  }
//---------------------------------------------------------------------------//
  DescriptorDX12 ShaderVisibleDescriptorHeapDX12::AllocateGlobalDescriptor(GlobalResourceType aType, const char* aDebugName)
  {
    ASSERT(myGlobalAllocators[aType].GetPageSize() != 0);

    uint64 offset;
    const PagedLinearAllocator::Page* page = myGlobalAllocators[aType].Allocate(1, 0, offset, aDebugName);
    ASSERT(page, "Failed allocating shader-visible descriptor. Consider increasing the max Global descriptor sizes");

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    cpuHandle.ptr = myGlobalDescriptorCpuHeapStart[aType].ptr + offset * myHandleIncrementSize;

    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    gpuHandle.ptr = myGlobalDescriptorGpuHeapStart[aType].ptr + offset * myHandleIncrementSize;

    return DescriptorDX12(cpuHandle, gpuHandle, myDesc.Type, aType, (uint)offset,true, true );
  }
//---------------------------------------------------------------------------//
  void ShaderVisibleDescriptorHeapDX12::FreeGlobalDescriptorAfterFrameDone(const DescriptorDX12& aDescriptor)
  {
    ASSERT(aDescriptor.myGlobalResourceType != GLOBAL_RESOURCE_NUM && aDescriptor.myIsShaderVisible && aDescriptor.myIsManagedByAllocator);
    ASSERT(myGlobalAllocators[aDescriptor.myGlobalResourceType].GetPageSize() != 0);

    const uint offset = (aDescriptor.myCpuHandle.ptr - myGlobalDescriptorCpuHeapStart[aDescriptor.myGlobalResourceType].ptr) / myHandleIncrementSize;
    const uint freeListIdx = Time::ourFrameIdx % ourNumGlobalFreeLists;
    myGlobalDescriptorsToFree[freeListIdx][aDescriptor.myGlobalResourceType].push_back(offset);
  }
//---------------------------------------------------------------------------//
}


#endif