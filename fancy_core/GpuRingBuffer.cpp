#include "fancy_core_precompile.h"
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
      myBuffer->Unmap(GpuResourceMapMode::WRITE_UNSYNCHRONIZED);
    myData = nullptr;
  }
//---------------------------------------------------------------------------//
  void GpuRingBuffer::Create(const GpuBufferProperties& someParameters, const char* aName /*= nullptr*/, const void* pInitialData)
  {
    Reset();
    
    if (myBuffer != nullptr && myData != nullptr)
      myBuffer->Unmap(GpuResourceMapMode::WRITE_UNSYNCHRONIZED);
    myData = nullptr;

    myBuffer = RenderCore::CreateBuffer(someParameters, aName, pInitialData);
    ASSERT(myBuffer != nullptr);

    myData = (uint8*)myBuffer->Map(GpuResourceMapMode::WRITE_UNSYNCHRONIZED);
    ASSERT(myData != nullptr);
  }
//---------------------------------------------------------------------------//
  uint64 GpuRingBuffer::GetFreeDataSize(uint64 anAlignment /*= 0*/) const
  {
    const uint64 alignedOffset = MathUtil::Align(myOffset, anAlignment);
    const uint64 bufferSize = myBuffer->GetByteSize();
    if (alignedOffset >= bufferSize)
      return 0;

    return bufferSize - alignedOffset;
  }
//---------------------------------------------------------------------------//
  bool GpuRingBuffer::AllocateAndWrite(const void* someData, uint64 aDataSize, uint64& anOffsetOut, uint64 anAlignment /*= 0*/)
  {
    ASSERT(myData != nullptr);

    if (GetFreeDataSize(anAlignment) < aDataSize)
      return false;

    const uint64 alignedOffset = MathUtil::Align(myOffset, anAlignment);
    anOffsetOut = alignedOffset;
    memcpy(myData + alignedOffset, someData, aDataSize);
    myOffset += MathUtil::Align(aDataSize, myBuffer->GetAlignment());
    return true;
  }
//---------------------------------------------------------------------------//
  bool GpuRingBuffer::Allocate(uint64 aDataSize, uint64& anOffsetOut, uint64 anAlignment /*= 0*/)
  {
    if (GetFreeDataSize(anAlignment) < aDataSize)
      return false;

    const uint64 alignedOffset = MathUtil::Align(myOffset, anAlignment);
    anOffsetOut = alignedOffset;
    myOffset += MathUtil::Align(aDataSize, myBuffer->GetAlignment());
    return true;
  }
//---------------------------------------------------------------------------//
}