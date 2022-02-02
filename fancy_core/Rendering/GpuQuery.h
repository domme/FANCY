#pragma once

#include "Common/FancyCoreDefines.h"
#include "RenderEnums.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct GpuQuery
  {
    GpuQuery() {}

    GpuQuery(GpuQueryType aType, uint anIndex, uint64 aFrame, CommandListType aCommandListType)
      : myFrame(aFrame)
      , myType(aType)
      , myIndexInHeap(anIndex)
      , myCommandListType(aCommandListType)
    {}

    uint64 myFrame = 0u;
    GpuQueryType myType = GpuQueryType::NUM;
    uint myIndexInHeap = 0u;
    CommandListType myCommandListType = CommandListType::Graphics;
    mutable bool myIsOpen = true;
  };
//---------------------------------------------------------------------------//
}
