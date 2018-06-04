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

    void Create(const GpuBufferCreationParams& clParameters, const void* pInitialData = nullptr);

    uint GetFreeDataSize() const;
    bool AppendData(void* someData, uint aDataSize, uint& anOffsetOut);
    void Reset() { myOffset = 0u; }
    const GpuBuffer* GetBuffer() const { return myBuffer.get(); }

  protected:
    SharedPtr<GpuBuffer> myBuffer;
    uint8* myData;
    uint myOffset;
  };
//---------------------------------------------------------------------------//
}
