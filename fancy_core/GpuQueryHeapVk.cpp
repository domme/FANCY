#include "fancy_core_precompile.h"
#include "GpuQueryHeapVk.h"

namespace Fancy
{
  GpuQueryHeapVk::GpuQueryHeapVk(GpuQueryType aQueryType, uint aNumQueries)
    : GpuQueryHeap(aQueryType, aNumQueries)
  {
    VK_MISSING_IMPLEMENTATION();
  }

  GpuQueryHeapVk::~GpuQueryHeapVk()
  {
    VK_MISSING_IMPLEMENTATION();
  }
}


