#pragma once

#include "FancyCoreDefines.h"
#include "RenderEnums.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct GpuQuery
  {
    GpuQuery(GpuQueryType aType, uint anIndex, uint64 aFrame)
      : myFrame(aFrame)
      , myType(aType)
      , myIndexInHeap(anIndex)
    {}

    uint64 myFrame;
    GpuQueryType myType;
    uint myIndexInHeap;
    mutable bool myIsOpen = true;
  };
//---------------------------------------------------------------------------//
  class GpuQueryHeap;

  struct GpuQueryRange
  {
    GpuQueryRange() {}

    GpuQueryRange(GpuQueryHeap* aHeap, uint aFirstQueryIndex, uint aNumQueries)
      : myHeap(aHeap)
      , myFirstQueryIndex(aFirstQueryIndex)
      , myNumQueries(aNumQueries)
    { }

    GpuQueryHeap* myHeap = nullptr;
    uint myFirstQueryIndex = 0u;
    uint myNumQueries = 0u;
    uint myNumUsedQueries = 0u;
  };
//---------------------------------------------------------------------------//
}
