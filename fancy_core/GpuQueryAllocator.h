#pragma once

#include "FancyCoreDefines.h"
#include "RenderEnums.h"
#include "GpuQuery.h"

namespace Fancy
{
  class GpuQueryHeap;

  class GpuQueryAllocator
  {
  public:
    GpuQueryAllocator(GpuQueryHeap* aHeap);
    ~GpuQueryAllocator() = default;

    void BeginFrame();
    GpuQuery Allocate();

    GpuQueryHeap* GetHeap() const { return myHeap.get(); }

  private:
    UniquePtr<GpuQueryHeap> myHeap;
    uint myNextFree = 0u;
  };
}



