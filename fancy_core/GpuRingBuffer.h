#pragma once

#include "FancyCoreDefines.h"
#include "GpuBuffer.h"
#include "Ptr.h"
#include "RenderEnums.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  class GpuRingBuffer
  {
  public:
    GpuRingBuffer();
    ~GpuRingBuffer();

    void Create(const GpuBufferProperties& someParameters, const char* aName = nullptr, const void* pInitialData = nullptr);

    uint64 GetFreeDataSize() const;
    bool AllocateAndWrite(const void* someData, uint64 aDataSize, uint64& anOffsetOut);
    bool Allocate(uint64 aDataSize, uint64& anOffsetOut);
    void Reset() { myOffset = 0u; }
    const GpuBuffer* GetBuffer() const { return myBuffer.get(); }
    uint8* GetData() const { return myData; }

  protected:
    SharedPtr<GpuBuffer> myBuffer;
    uint8* myData;
    uint64 myOffset;
  };
//---------------------------------------------------------------------------//
}
