#include "fancy_core_precompile.h"
#include "TextureReadbackTask.h"

#include "RenderCore.h"

namespace Fancy
{
  ReadbackBufferAllocation::~ReadbackBufferAllocation()
  {
    if (myBuffer != nullptr && myBlockSize != 0u)
    {
      RenderCore::FreeReadbackBuffer(myBuffer, myBlockSize, myOffsetToBlock);
    }
  }
}
