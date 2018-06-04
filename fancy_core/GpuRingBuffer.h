#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  class GpuRingBuffer
  {
  public:
    GpuRingBuffer();
    ~GpuRingBuffer();

    void Create(const GpuBufferCreationParams& someParameters, GpuResoruceLockOption aLockOption, const void* pInitialData = nullptr);

    uint GetFreeDataSize() const;
    bool Write(void* someData, uint aDataSize, uint& anOffsetOut);
    void Reset() { myOffset = 0u; }
    const GpuBuffer* GetBuffer() const { return myBuffer.get(); }

  protected:
    SharedPtr<GpuBuffer> myBuffer;
    uint8* myData;
    uint myOffset;
    GpuResoruceLockOption myLockType;
  };
//---------------------------------------------------------------------------//
}
