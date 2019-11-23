#pragma once

#include "TextureProperties.h"
#include "TextureData.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  class GpuBuffer;
//---------------------------------------------------------------------------//
  struct ReadbackBufferAllocation
  {
    ~ReadbackBufferAllocation();
    GpuBuffer* myBuffer = nullptr;
    uint64 myOffsetToBlock = 0u;
    uint64 myBlockSize = 0u;
  };
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  class TextureReadbackTask
  {
  public:
    TextureReadbackTask(TextureProperties someTextureProperties, SubresourceRange aSubresourceRange, 
      SharedPtr<ReadbackBufferAllocation> aBufferAllocation, uint64 aFence, uint64 aRowAlignment)
      : myTextureProperties(std::move(someTextureProperties))
      , mySubresourceRange(std::move(aSubresourceRange))
      , myBufferAllocation(std::move(aBufferAllocation))
      , myFence(aFence)
      , myRowAlignment(aRowAlignment)
    { }

    bool GetData(TextureData& aDataOut) const;
    bool IsCompleted() const;
    void WaitForCompletion() const;
    GpuBuffer* GetBuffer() const { return myBufferAllocation ? myBufferAllocation->myBuffer : nullptr; }

  private:
    TextureProperties myTextureProperties;
    SubresourceRange mySubresourceRange;
    SharedPtr<ReadbackBufferAllocation> myBufferAllocation;
    uint64 myFence;
    uint64 myRowAlignment;
    
  };
//---------------------------------------------------------------------------//
}
