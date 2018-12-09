#include "GpuBuffer.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  MappedBufferData::~MappedBufferData()
  {
    if (!myIsPersistantMap)
      myBuffer->Unmap(myMapMode, myOffset, mySize);
  }
//---------------------------------------------------------------------------//
  GpuBuffer::GpuBuffer()
    : GpuResource(GpuResourceCategory::BUFFER)
    , myAlignment(0u)
  {
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  bool GpuBuffer::Map(MappedBufferData& aMappedDataOut, bool aIsPersistent, GpuResourceMapMode aMapMode, uint64 anOffset, uint64 aSize) const
  {
    void* mappedData = Map(aMapMode, anOffset, aSize);
    if (mappedData == nullptr)
      return false;

    aMappedDataOut.myBuffer = this;
    aMappedDataOut.myData = mappedData;
    aMappedDataOut.myOffset = anOffset;
    aMappedDataOut.mySize = aSize;
    aMappedDataOut.myMapMode = aMapMode;
    aMappedDataOut.myIsPersistantMap = aIsPersistent;
    return true;
  }
//---------------------------------------------------------------------------//
}