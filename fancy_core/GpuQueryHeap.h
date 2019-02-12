#pragma once

#include "RenderEnums.h"

namespace Fancy
{
  class GpuQueryHeap
  {
  public:
    GpuQueryHeap(QueryType aQueryType, uint aNumQueries);
    virtual ~GpuQueryHeap() = 0;

  protected:
    QueryType myType;
    uint myNumQueries;
  };
}



