#pragma once

#include "GpuResourceViewSet.h"
#include "DescriptorDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
  class GpuResourceViewSetDX12 : public GpuResourceViewSet
  {
  public:
    GpuResourceViewSetDX12(const eastl::span<GpuResourceViewSetElement>& someResources);
    ~GpuResourceViewSetDX12() override = default;

    void UpdateDescriptors() const override;

    DescriptorDX12 GetFirstDescriptor() const { return myFirstConstantDynamicDescriptor; }

  protected:
    DescriptorDX12 myFirstConstantDynamicDescriptor;
  };
}

#endif