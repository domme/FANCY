#pragma once

#include "GpuResourceView.h"
#include "EASTL/variant.h"

namespace Fancy
{
  struct GpuResourceViewRange
  {
    eastl::fixed_vector<SharedPtr<GpuResourceView>, 1> myResources;
    bool myIsUnbounded = false;
  };

  class GpuResourceViewSet
  {
  public:
    GpuResourceViewSet(const eastl::span<GpuResourceViewRange>& someRanges);
    virtual ~GpuResourceViewSet() = default;

    bool HasResourceView(const GpuResourceView* aView) const;
    const eastl::fixed_vector<GpuResourceViewRange, 32>& GetRanges() const { return myRanges; }

  protected:
    eastl::fixed_vector<GpuResourceViewRange, 32> myRanges;
  };
}
