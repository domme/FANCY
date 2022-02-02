#pragma once

#if FANCY_ENABLE_VK

#include "Rendering/GpuQueryHeap.h"

#include "VkPrerequisites.h"

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