#pragma once

#include "GpuBuffer.h"
#include "Common/GrowingList.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  class GpuReadbackBuffer
  {
  public:
    explicit GpuReadbackBuffer(uint64 aSize);
    ~GpuReadbackBuffer();

    GpuBuffer* AllocateBlock(uint64 aSize, uint anOffsetAlignment, uint64& anOffsetOut);
    bool FreeBlock(GpuBuffer* aBuffer, uint64 anOffsetToBlock, uint64 aBlockSize);
    bool IsEmpty() const { return myUsedBlocks.IsEmpty(); }
    const uint64 GetFreeSize() const { return myFreeSize; }

  private:
    struct Block
    {
      uint64 myOffset;
      uint64 mySize;
    };

    GrowingList<Block, 64> myUsedBlocks;
    SharedPtr<GpuBuffer> myBuffer;
    uint64 myNextFree = 0u;
    uint64 myFreeSize = 0u;
  };
//---------------------------------------------------------------------------//
}



