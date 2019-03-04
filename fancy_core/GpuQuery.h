#pragma once

#include "FancyCoreDefines.h"
#include "RenderEnums.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct GpuQuery
  {
    GpuQuery() {}

    GpuQuery(GpuQueryType aType, uint anIndex, uint64 aFrame)
      : myFrame(aFrame)
      , myType(aType)
      , myIndexInHeap(anIndex)
    {}

    uint64 myFrame = 0u;
    GpuQueryType myType = GpuQueryType::NUM;
    uint myIndexInHeap = 0u;
    mutable bool myIsOpen = true;
  };
//---------------------------------------------------------------------------//
}
