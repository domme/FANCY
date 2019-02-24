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

    GpuQueryType myType = GpuQueryType::TIMESTAMP;
    uint myNumQueries = 0u;
  };
//---------------------------------------------------------------------------//
}