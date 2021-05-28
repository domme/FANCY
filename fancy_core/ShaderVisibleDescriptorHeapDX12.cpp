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
      default: ASSERT(false); return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
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
  ShaderVisibleDescriptorHeapDX12::ShaderVisibleDescriptorHeapDX12(const RenderPlatformProperties& someProperties)
    : myNumGlobalDescriptors{
      someProperties.myNumGlobalTextures,
      someProperties.myNumGlobalTextures,
      someProperties.myNumGlobalBuffers,
      someProperties.myNumGlobalBuffers,
      someProperties.myNumGlobalSamplers }
    , myAllocators{
      PagedLinearAllocator(someProperties.myNumGlobalTextures),
      PagedLinearAllocator(someProperties.myNumGlobalTextures),
      PagedLinearAllocator(someProperties.myNumGlobalBuffers),
      PagedLinearAllocator(someProperties.myNumGlobalBuffers),
      PagedLinearAllocator(someProperties.myNumGlobalSamplers)
    }
  {

    // Create resource Heap
    uint numResourceDescriptors = 0;
    for (uint i = 0; i < GLOBAL_RESOURCE_NUM_NOSAMPLER; ++i)
      numResourceDescriptors += myNumGlobalDescriptors[i];

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    heapDesc.NumDescriptors = numResourceDescriptors;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.NodeMask = 0u;

    ID3D12Device* device = RenderCore::GetPlatformDX12()->GetDevice();
    ASSERT_HRESULT(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&myResourceHeap)));
    myResourceHandleIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    uint offset = 0;
    for (uint i = 0; i < GLOBAL_RESOURCE_NUM_NOSAMPLER; ++i)
    {
      ASSERT(myAllocators[i].IsEmpty());

      myGlobalDescriptorCpuHeapStart[i] = myResourceHeap->GetCPUDescriptorHandleForHeapStart();;
      myGlobalDescriptorCpuHeapStart[i].ptr += offset * myResourceHandleIncrementSize;

      myGlobalDescriptorGpuHeapStart[i] = myResourceHeap->GetGPUDescriptorHandleForHeapStart();;
      myGlobalDescriptorGpuHeapStart[i].ptr += offset * myResourceHandleIncrementSize;

      offset += myNumGlobalDescriptors[i];
    }

    // Create sampler heap
    heapDesc.NumDescriptors = myNumGlobalDescriptors[GLOBAL_RESOURCE_SAMPLER];
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;

    ASSERT_HRESULT(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mySamplerHeap)));
    mySamplerHandleIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

    ASSERT(myAllocators[GLOBAL_RESOURCE_SAMPLER].IsEmpty());

    myGlobalDescriptorCpuHeapStart[GLOBAL_RESOURCE_SAMPLER] = mySamplerHeap->GetCPUDescriptorHandleForHeapStart();;
    myGlobalDescriptorGpuHeapStart[GLOBAL_RESOURCE_SAMPLER] = mySamplerHeap->GetGPUDescriptorHandleForHeapStart();;

    // Initialize heaps with null-descriptors
    for (uint i = 0; i < GLOBAL_RESOURCE_NUM; ++i)
    {
      const uint handleIncrementSize = i == GLOBAL_RESOURCE_SAMPLER ? mySamplerHandleIncrementSize : myResourceHandleIncrementSize;

      if (myNumGlobalDescriptors[i] > 0)
      {
        const GlobalResourceType GlobalType = static_cast<GlobalResourceType>(i);

        const DescriptorDX12& nullDescriptor =
          RenderCore::GetPlatformDX12()->GetNullDescriptor(Private::GetDescriptorRangeType(GlobalType), Private::GetResourceDimension(GlobalType));

        for (uint k = 0; k < myNumGlobalDescriptors[i]; ++k)
        {
          D3D12_CPU_DESCRIPTOR_HANDLE dstDescriptor = myGlobalDescriptorCpuHeapStart[i];
          dstDescriptor.ptr += k * handleIncrementSize;

          device->CopyDescriptorsSimple(1, dstDescriptor, nullDescriptor.myCpuHandle, 
            i == GLOBAL_RESOURCE_SAMPLER ? D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER : D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }
      }
    }
  }
//---------------------------------------------------------------------------//
  void ShaderVisibleDescriptorHeapDX12::ProcessGlobalDescriptorFrees()
  {
    if (Time::ourFrameIdx < ourNumGlobalFreeLists - 1)
      return;

    const uint64 frameIdxToProcess = Time::ourFrameIdx - (static_cast<uint64>(ourNumGlobalFreeLists) - 1u);
    ASSERT(RenderCore::IsFrameDone(frameIdxToProcess));

    const uint freeListToProcess = frameIdxToProcess % ourNumGlobalFreeLists;
    for (const DescriptorToFree& descriptorToFree : myDescriptorsToFree[freeListToProcess])
    {
      const GlobalResourceType resourceType = descriptorToFree.myType;
      myAllocators[resourceType].Free({ descriptorToFree.myIndex, descriptorToFree.myIndex + 1 });

#if FANCY_HEAVY_DEBUG
      // Replace with null descriptor
      // For debugging, this could also become a special error-resource for detecting if something deleted is accessed in a shader.

      const DescriptorDX12& nullDescriptor =
        RenderCore::GetPlatformDX12()->GetNullDescriptor(Private::GetDescriptorRangeType(resourceType), Private::GetResourceDimension(resourceType));

      D3D12_CPU_DESCRIPTOR_HANDLE dst;
      dst.ptr = myGlobalDescriptorCpuHeapStart[resourceType].ptr + descriptorToFree.myIndex * (resourceType == GLOBAL_RESOURCE_SAMPLER ? mySamplerHandleIncrementSize : myResourceHandleIncrementSize);
      RenderCore::GetPlatformDX12()->GetDevice()->CopyDescriptorsSimple(1, dst, nullDescriptor.myCpuHandle, 
        resourceType == GLOBAL_RESOURCE_SAMPLER ? D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER : D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
#endif
    }

    myDescriptorsToFree[freeListToProcess].clear();
  }
//---------------------------------------------------------------------------//
  DescriptorDX12 ShaderVisibleDescriptorHeapDX12::AllocateDescriptor(GlobalResourceType aType, const char* aDebugName)
  {
    ASSERT(myAllocators[aType].GetPageSize() != 0);

    uint64 offset;
    const PagedLinearAllocator::Page* page = myAllocators[aType].Allocate(1, 1, offset, aDebugName);
    ASSERT(page, "Failed allocating shader-visible descriptor. Consider increasing the max Global descriptor sizes");

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    cpuHandle.ptr = myGlobalDescriptorCpuHeapStart[aType].ptr + offset * GetHandleIncementSize(aType);

    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    gpuHandle.ptr = myGlobalDescriptorGpuHeapStart[aType].ptr + offset * GetHandleIncementSize(aType);

    return DescriptorDX12(cpuHandle, gpuHandle, GetHeapType(aType), aType, (uint)offset,true, true );
  }
//---------------------------------------------------------------------------//
  void ShaderVisibleDescriptorHeapDX12::FreeDescriptorAfterFrameDone(const DescriptorDX12& aDescriptor)
  {
    ASSERT(aDescriptor.myGlobalResourceType != GLOBAL_RESOURCE_NUM && aDescriptor.myIsShaderVisible && aDescriptor.myIsManagedByAllocator);
    ASSERT(myAllocators[aDescriptor.myGlobalResourceType].GetPageSize() != 0);
    ASSERT(aDescriptor.myGlobalResourceIndex < myNumGlobalDescriptors[aDescriptor.myGlobalResourceType]);

    const uint freeListIdx = Time::ourFrameIdx % ourNumGlobalFreeLists;
    myDescriptorsToFree[freeListIdx].push_back({ aDescriptor.myGlobalResourceType, aDescriptor.myGlobalResourceIndex });
  }
//---------------------------------------------------------------------------//
}


#endif