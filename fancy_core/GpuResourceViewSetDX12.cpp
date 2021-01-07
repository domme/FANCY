#include "fancy_core_precompile.h"
#include "GpuResourceViewSetDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"
#include "DynamicDescriptorHeapDX12.h"
#include "GpuResourceViewDataDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
//---------------------------------------------------------------------------//
  GpuResourceViewSetDX12::GpuResourceViewSetDX12(const eastl::span<GpuResourceViewRange>& someRanges)
    : GpuResourceViewSet(someRanges)
  {
    RenderCore_PlatformDX12* platformDx12 = RenderCore::GetPlatformDX12();
    DynamicDescriptorHeapDX12* dynamicHeap = platformDx12->GetDynamicDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    uint numDescriptorsInTable = 0u;
    for (const GpuResourceViewRange& range : myRanges)
      for (const SharedPtr<GpuResourceView>& view : range.myResources)
        ++numDescriptorsInTable;

    myFirstConstantDynamicDescriptor = dynamicHeap->AllocateConstantDescriptorRange(numDescriptorsInTable);

    eastl::fixed_vector<D3D12_CPU_DESCRIPTOR_HANDLE, 256> srcDescriptors;
    srcDescriptors.reserve(numDescriptorsInTable);

    eastl::fixed_vector<uint, 256> srcRegionSizes;
    srcRegionSizes.resize(numDescriptorsInTable, 1u);

    myDstStatesPerRange.reserve(myRanges.size());
    myDescriptorTypePerRange.reserve(myRanges.size());

    for (const GpuResourceViewRange& range : myRanges)
    {
      D3D12_DESCRIPTOR_RANGE_TYPE rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
      for (const SharedPtr<GpuResourceView>& resource : range.myResources)
      {
        // The first valid resource determines the range type
        if (resource)
        {
          rangeType = RenderCore_PlatformDX12::GetDescriptorRangeType(resource->GetType());
          break;
        }
      }

      myDescriptorTypePerRange.push_back(rangeType);

      D3D12_RESOURCE_STATES dstStates = D3D12_RESOURCE_STATE_COMMON;
      switch (rangeType)
      {
      case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
        dstStates = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        break;
      case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
        dstStates = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        break;
      case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
        dstStates = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        break;
      default: ASSERT(false);
      }
      myDstStatesPerRange.push_back(dstStates);

      for (const SharedPtr<GpuResourceView>& resource : range.myResources)
      {
        if (resource != nullptr)
        {
          const GpuResourceViewDataDX12& viewDataDx12 = eastl::any_cast<const GpuResourceViewDataDX12&>(resource->myNativeData);
          srcDescriptors.push_back(viewDataDx12.myDescriptor.myCpuHandle);
        }
        else // Not bound, pick an appropriate null descriptor
        {
          srcDescriptors.push_back(platformDx12->GetNullDescriptor(rangeType).myCpuHandle);
        }
      }
    }
    
    D3D12_CPU_DESCRIPTOR_HANDLE dstStart = myFirstConstantDynamicDescriptor.myCpuHandle;

    platformDx12->GetDevice()->CopyDescriptors(1u, &dstStart, &numDescriptorsInTable,
      numDescriptorsInTable, srcDescriptors.data(), srcRegionSizes.data(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  }
//---------------------------------------------------------------------------//
}

#endif