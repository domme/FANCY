#pragma once

#include "GpuBuffer.h"
#include "GrowingList.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  class GpuReadbackBuffer
  {
  public:
    explicit GpuReadbackBuffer(uint64 aSize);
    ~GpuReadbackBuffer();

    GpuBuffer* AllocateBlock(uint64 aSize, uint64& anOffsetOut);
    bool FreeBlock(GpuBuffer* aBuffer, uint64 anOffsetToBlock, uint64 aBlockSize);

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



