#pragma once

#include "Common/FancyCoreDefines.h"
#include "Common/Ptr.h"
#include "GpuBuffer.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  class GpuRingBuffer
  {
  public:
    GpuRingBuffer();
    ~GpuRingBuffer();

    void Create(const GpuBufferProperties& someParameters, const char* aName = nullptr, const void* pInitialData = nullptr);

    uint64 GetFreeDataSize(uint64 anAlignment = 0) const;
    bool AllocateAndWrite(const void* someData, uint64 aDataSize, uint64& anOffsetOut, uint64 anAlignment = 0);
    bool Allocate(uint64 aDataSize, uint64& anOffsetOut, uint64 anAlignment = 0);
    void Reset() { myOffset = 0u; }
    GpuBuffer* GetBuffer() const { return myBuffer.get(); }
    uint8* GetData() const { return myData; }

  protected:
    SharedPtr<GpuBuffer> myBuffer;
    uint8* myData;
    uint64 myOffset;
  };
//---------------------------------------------------------------------------//
}
