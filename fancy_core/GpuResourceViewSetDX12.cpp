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
  GpuResourceViewSetDX12::GpuResourceViewSetDX12(const eastl::span<GpuResourceViewSetElement>& someResources)
    : GpuResourceViewSet(someResources)
  {
    RenderCore_PlatformDX12* platformDx12 = RenderCore::GetPlatformDX12();
    DynamicDescriptorHeapDX12* dynamicHeap = platformDx12->GetDynamicDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    myFirstConstantDynamicDescriptor = dynamicHeap->AllocateConstantDescriptorRange((uint) someResources.size());

    GpuResourceViewSetDX12::UpdateDescriptors();
  }
//---------------------------------------------------------------------------//
  void GpuResourceViewSetDX12::UpdateDescriptors() const
  {
    if (!myIsDirty)
      return;

    myIsDirty = false;

    // The descriptors might still be in use by the GPU. Since UpdateDescriptors() is assumed to be very infrequent,
    // we wait for the GPU to be idle instead of double-buffering with a new allocation.
    RenderCore::WaitForIdle(CommandListType::Graphics);
    RenderCore::WaitForIdle(CommandListType::Compute);

    RenderCore_PlatformDX12* platformDx12 = RenderCore::GetPlatformDX12();

    uint numDescriptorsInTable = (uint) myResources.size();

    eastl::fixed_vector<D3D12_CPU_DESCRIPTOR_HANDLE, 256> srcDescriptors;
    srcDescriptors.reserve(numDescriptorsInTable);

    eastl::fixed_vector<uint, 256> srcRegionSizes;
    srcRegionSizes.resize(numDescriptorsInTable, 1u);

    for (const GpuResourceViewSetElement& resource : myResources)
    {
      if (resource.myView != nullptr)
      {
        const GpuResourceViewDataDX12& viewDataDx12 = eastl::any_cast<const GpuResourceViewDataDX12&>(resource.myView->myNativeData);
        srcDescriptors.push_back(viewDataDx12.myDescriptor.myCpuHandle);
      }
      else // Not bound, pick an appropriate null descriptor
      {
        D3D12_DESCRIPTOR_RANGE_TYPE rangeType = RenderCore_PlatformDX12::GetDescriptorRangeType(resource.myType);
        srcDescriptors.push_back(platformDx12->GetNullDescriptor(rangeType).myCpuHandle);
      }
    }

    D3D12_CPU_DESCRIPTOR_HANDLE dstStart = myFirstConstantDynamicDescriptor.myCpuHandle;

    platformDx12->GetDevice()->CopyDescriptors(1u, &dstStart, &numDescriptorsInTable,
      numDescriptorsInTable, srcDescriptors.data(), srcRegionSizes.data(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  }
//---------------------------------------------------------------------------//
}

#endif