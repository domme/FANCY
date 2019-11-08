#pragma once
#include "GpuQueryHeap.h"
#include "VkPrerequisites.h"

namespace Fancy
{
  class GpuQueryHeapVk : public GpuQueryHeap
  {
  public:
    GpuQueryHeapVk(GpuQueryType aQueryType, uint aNumQueries);
    ~GpuQueryHeapVk() override;
  };
}



