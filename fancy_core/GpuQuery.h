#pragma once

#include "FancyCoreDefines.h"
#include "RenderEnums.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct GpuQuery
  {
    GpuQueryType myType = GpuQueryType::NUM;
    uint myIndexInHeap = 0u;
    uint64 myFrame = 0u;
  };
//---------------------------------------------------------------------------//
  struct GpuQueryRange
  {
    GpuQueryType myType = GpuQueryType::NUM;
    uint myFirstQueryIndex = 0u;
    uint myNumQueries = 0u;
    uint myNumUsedQueries = 0u;
  };
//---------------------------------------------------------------------------//
}
