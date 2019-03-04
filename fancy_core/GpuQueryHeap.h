#pragma once

#include "RenderEnums.h"
#include "GpuQuery.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  class GpuQueryHeap
  {
  public:
    GpuQueryHeap(GpuQueryType aQueryType, uint aNumQueries);
    virtual ~GpuQueryHeap();

    void Reset(uint64 aFrame) { myNextFreeQuery = 0u; myLastUsedFrame = aFrame; }
    uint Allocate(uint aNumQueries);

    uint64 myLastUsedFrame = UINT64_MAX;
    GpuQueryType myType = GpuQueryType::TIMESTAMP;
    uint myNumQueries = 0u;
    uint myNextFreeQuery = 0u;
  };
//---------------------------------------------------------------------------//
}