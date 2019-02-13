#pragma once

#include "FancyCoreDefines.h"
#include "RenderEnums.h"

namespace Fancy
{
  class GpuQueryHeap;

  struct GpuQuery
  {
    QueryType myType = QueryType::NUM;
    uint myIndexInHeap = UINT_MAX;
    uint64 myFrame = UINT64_MAX;
    GpuQueryHeap* myHeap = nullptr;
  };
}
