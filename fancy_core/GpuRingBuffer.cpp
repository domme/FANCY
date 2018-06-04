#include "GpuRingBuffer.h"
#include "GpuBuffer.h"
#include "RenderCore.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  GpuRingBuffer::GpuRingBuffer() 
    : myData(nullptr)
    , myOffset(0u)
  {
    
  }
//---------------------------------------------------------------------------//
  GpuRingBuffer::~GpuRingBuffer()
  {
    if (myBuffer != nullptr && myData != nullptr)
      myBuffer->Unlock();
    myData = nullptr;
  }
//---------------------------------------------------------------------------//
  void GpuRingBuffer::Create(const GpuBufferCreationParams& clParameters, const void* pInitialData)
  {
    Reset();
    
    if (myBuffer != nullptr && myData != nullptr)
      myBuffer->Unlock();
    myData = nullptr;

    myBuffer = RenderCore::CreateBuffer(clParameters, pInitialData);
    ASSERT(myBuffer != nullptr);

    GpuResoruceLockOption lockType = (clParameters.uAccessFlags & (uint)GpuResourceAccessFlags::WRITE) ? GpuResoruceLockOption::WRITE;

    myData = (uint8*) myBuffer->Lock( );
    ASSERT(myData != nullptr);
  }
//---------------------------------------------------------------------------//
  uint GpuRingBuffer::GetFreeDataSize() const
  {
    return myBuffer->GetSizeBytes() - myOffset;
  }
//---------------------------------------------------------------------------//
  bool GpuRingBuffer::AppendData(void* someData, uint aDataSize, uint& anOffsetOut)
  {
    if (GetFreeDataSize() < aDataSize)
      return false;

    anOffsetOut = myOffset;
    memcpy(myData + myOffset, someData, aDataSize);
    myOffset += MathUtil::Align(aDataSize, myBuffer->GetAlignment());
    return true;
  }
//---------------------------------------------------------------------------//
}
