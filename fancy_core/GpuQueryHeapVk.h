#pragma once
#include "GpuQueryHeap.h"
#include "VkPrerequisites.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
//---------------------------------------------------------------------------//
  class GpuQueryHeapVk : public GpuQueryHeap
  {
  public:
    GpuQueryHeapVk(GpuQueryType aQueryType, uint aNumQueries);
    ~GpuQueryHeapVk() override;

    void Reset(uint64 aFrame) override;

    VkQueryPool GetQueryPool() const { return myQueryPool; }

  private:
    VkQueryPool myQueryPool;
  };
//---------------------------------------------------------------------------//
}

#endif