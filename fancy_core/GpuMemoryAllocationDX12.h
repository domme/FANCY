#pragma once

#include "DX12Prerequisites.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct GpuMemoryAllocationDX12
  {
    GpuMemoryAllocationDX12() : myHeap(nullptr), myOffsetInHeap(0u), mySize(0u) {}
    ID3D12Heap* myHeap;
    uint64 myOffsetInHeap;
    uint64 mySize;
  };
//---------------------------------------------------------------------------//
}

