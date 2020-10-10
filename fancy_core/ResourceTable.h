#pragma once

#include "GpuResource.h"

namespace Fancy
{
  class ResourceTable
  {
  public:
    ResourceTable();
    ~ResourceTable();

  protected:
    eastl::fixed_vector<SharedPtr<GpuResource>, 32> myResources;
  };
}
