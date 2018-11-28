#include "GpuRingBuffer.h"
#include "GpuBuffer.h"
#include "RenderCore.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  GpuRingBuffer::GpuRingBuffer() 
    : myData(nullptr)
    , myOffset(0u)
    , myLockType(GpuResourceMapMode::NUM)
  {
    
  }
//---------------------------------------------------------------------------//
  GpuRingBuffer::~GpuRingBuffer()
  {
    if (myBuffer != nullptr && myData != nullptr)
      myBuffer->Unmap(myLockType);
    myData = nullptr;
  }
//---------------------------------------------------------------------------//
  void GpuRingBuffer::Create(const GpuBufferProperties& someParameters, GpuResourceMapMode aLockOption, const char* aName /*= nullptr*/, const void* pInitialData)
  {
    Reset();
    
    if (myBuffer != nullptr && myData != nullptr)
      myBuffer->Unmap(myLockType);
    myData = nullptr;

    myBuffer = RenderCore::CreateBuffer(someParameters, aName, pInitialData);
    ASSERT(myBuffer != nullptr);

    myData = (uint8*) myBuffer->Map(aLockOption);
    ASSERT(myData != nullptr);

    myLockType = aLockOption;
  }
//---------------------------------------------------------------------------//
  uint64 GpuRingBuffer::GetFreeDataSize() const
  {
    return myBuffer->GetByteSize() - myOffset;
  }
//---------------------------------------------------------------------------//
  bool GpuRingBuffer::AllocateAndWrite(const void* someData, uint64 aDataSize, uint64& anOffsetOut)
  {
    ASSERT(myLockType == GpuResourceMapMode::WRITE_UNSYNCHRONIZED);

    if (GetFreeDataSize() < aDataSize)
      return false;

    anOffsetOut = myOffset;
    memcpy(myData + myOffset, someData, aDataSize);
    myOffset += MathUtil::Align(aDataSize, myBuffer->GetAlignment());
    return true;
  }
//---------------------------------------------------------------------------//
  bool GpuRingBuffer::Allocate(uint64 aDataSize, uint64& anOffsetOut)
  {
    if (GetFreeDataSize() < aDataSize)
      return false;

    anOffsetOut = myOffset;
    myOffset += MathUtil::Align(aDataSize, myBuffer->GetAlignment());
    return true;
  }
//---------------------------------------------------------------------------//
}