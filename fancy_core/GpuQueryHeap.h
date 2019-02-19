#pragma once

#include "RenderEnums.h"
#include "GpuQuery.h"

namespace Fancy
{
  class GpuQueryHeap
  {
  public:
    GpuQueryHeap(QueryType aQueryType, uint aNumQueries);
    virtual ~GpuQueryHeap();

    GpuQuery Allocate();
    void Reset();

    QueryType myType;
    uint myNumQueries = 0u;;
    uint myNextFree = 0u;
  };
}



