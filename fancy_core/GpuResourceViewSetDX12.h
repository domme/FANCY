#pragma once

#include "GpuResourceViewSet.h"
#include "DescriptorDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
  class GpuResourceViewSetDX12 : public GpuResourceViewSet
  {
  public:
    GpuResourceViewSetDX12(const eastl::span<GpuResourceViewRange>& someRanges);
    DescriptorDX12 GetFirstDescriptor() const { return myFirstConstantDynamicDescriptor; }
    D3D12_RESOURCE_STATES GetDstStateForRange(uint aRangeIdx) const { return myDstStatesPerRange[aRangeIdx]; }
    D3D12_DESCRIPTOR_RANGE_TYPE GetDescriptorRangeType(uint aRangeIdx) const { return myDescriptorTypePerRange[aRangeIdx]; }

  protected:
    DescriptorDX12 myFirstConstantDynamicDescriptor;
    eastl::fixed_vector<D3D12_RESOURCE_STATES, 32> myDstStatesPerRange;
    eastl::fixed_vector<D3D12_DESCRIPTOR_RANGE_TYPE, 32> myDescriptorTypePerRange;
  };
}

#endif