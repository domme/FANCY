#include "fancy_core_precompile.h"
#include "GpuReadbackBuffer.h"
#include "RenderCore.h"

namespace Fancy
{
  GpuReadbackBuffer::GpuReadbackBuffer(uint64 aSize)
  {
    GpuBufferProperties props;
    props.myBindFlags = (uint) GpuBufferBindFlags::NONE;
    props.myCpuAccess = CpuMemoryAccessType::CPU_READ;
    props.myNumElements = 1u;
    props.myElementSizeBytes = aSize;
    props.myIsShaderWritable = false;

    static int bufferCounter = 0;
    myBuffer = RenderCore::CreateBuffer(props, StaticString<32>("Readback buffer %d", bufferCounter++));
    ASSERT(myBuffer != nullptr);

    myFreeSize = aSize;
  }

  GpuReadbackBuffer::~GpuReadbackBuffer()
  {
    ASSERT(myUsedBlocks.IsEmpty(), "Destructing readback-buffer %s while there are still %d blocks in use",
      myBuffer->myName.c_str(), myUsedBlocks.Size());
  }

  GpuBuffer* GpuReadbackBuffer::AllocateBlock(uint64 aSize, uint anOffsetAlignment, uint64& anOffsetOut)
  {
    if (myFreeSize < aSize)
      return nullptr;

    const uint64 alignedNextFree = MathUtil::Align(myNextFree, anOffsetAlignment);
    const uint64 requiredSize = aSize + (alignedNextFree - myNextFree);

    if (myFreeSize < requiredSize)
      return nullptr;

    anOffsetOut = alignedNextFree;
    myUsedBlocks.Add({ alignedNextFree, aSize });

    myNextFree = alignedNextFree + aSize;
    myFreeSize -= requiredSize;

    return myBuffer.get();
  }

  bool GpuReadbackBuffer::FreeBlock(GpuBuffer* aBuffer, uint64 anOffsetToBlock, uint64 aBlockSize)
  {
    if (myBuffer.get() != aBuffer)
      return false;

    for (auto it = myUsedBlocks.Begin(); it != myUsedBlocks.Invalid(); ++it)
    {
      const Block& block = *it;
      if (block.myOffset == anOffsetToBlock)
      {
        ASSERT(block.mySize == aBlockSize);

        myUsedBlocks.RemoveGetPrev(it);
        return true;
      }
    }

    ASSERT(false, "Unable to free block (offset %d, size %d) from buffer %s",
      anOffsetToBlock, aBlockSize, myBuffer->myName.c_str());
    return false;
  }
}


