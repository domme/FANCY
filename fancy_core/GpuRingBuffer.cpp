#include "GpuRingBuffer.h"
#include "GpuBuffer.h"
#include "RenderCore.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  GpuRingBuffer::GpuRingBuffer() 
    : myData(nullptr)
    , myOffset(0u)
    , myLockType(GpuResoruceLockOption::NUM)
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
  void GpuRingBuffer::Create(const GpuBufferProperties& someParameters, GpuResoruceLockOption aLockOption, const char* aName /*= nullptr*/, const void* pInitialData)
  {
    Reset();
    
    if (myBuffer != nullptr && myData != nullptr)
      myBuffer->Unlock();
    myData = nullptr;

    myBuffer = RenderCore::CreateBuffer(someParameters, aName, pInitialData);
    ASSERT(myBuffer != nullptr);

    myData = (uint8*) myBuffer->Lock(aLockOption);
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
    ASSERT(myLockType == GpuResoruceLockOption::WRITE || myLockType == GpuResoruceLockOption::READ_WRITE);

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