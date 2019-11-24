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
  class ReadbackTask
  {
  public:
    ReadbackTask(SharedPtr<ReadbackBufferAllocation> aBufferAllocation, uint64 aFence)
      : myBufferAllocation(std::move(aBufferAllocation))
      , myFence(std::move(aFence))
    { }

    void GetRawData(DynamicArray<uint8>& aDataOut);
    bool IsCompleted() const;
    void Wait() const;
    GpuBuffer* GetBuffer() const { return myBufferAllocation ? myBufferAllocation->myBuffer : nullptr; }

  protected:
    SharedPtr<ReadbackBufferAllocation> myBufferAllocation;
    uint64 myFence;
  };
//---------------------------------------------------------------------------//
  class TextureReadbackTask : public ReadbackTask
  {
  public:
    TextureReadbackTask(TextureProperties someTextureProperties, SubresourceRange aSubresourceRange, 
      SharedPtr<ReadbackBufferAllocation> aBufferAllocation, uint64 aFence)
      : ReadbackTask(std::move(aBufferAllocation), std::move(aFence))
      , myTextureProperties(std::move(someTextureProperties))
      , mySubresourceRange(std::move(aSubresourceRange))
    { }

    void GetData(TextureData& aDataOut) const;

  private:
    TextureProperties myTextureProperties;
    SubresourceRange mySubresourceRange;
  };
//---------------------------------------------------------------------------//
}
