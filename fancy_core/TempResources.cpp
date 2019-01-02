#include "fancy_core_precompile.h"
#include "TempResources.h"
#include "TempResourcePool.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  TempResourceKeepAlive::~TempResourceKeepAlive()
  {
    myPool->FreeResource(myResource, myBucketHash);
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  MappedTempBuffer::MappedTempBuffer(const TempBufferResource& aResource, GpuResourceMapMode aMapMode, uint64 aSize)
    : myTempBuffer(aResource)
    , myMapMode(aMapMode)
    , mySize(aSize)
  {
    myMappedData = myTempBuffer.myBuffer->Map(myMapMode, 0u, mySize);
  }
//---------------------------------------------------------------------------//
  MappedTempBuffer::~MappedTempBuffer()
  {
    Unmap();
  }
//---------------------------------------------------------------------------//
  void MappedTempBuffer::Unmap()
  {
    myTempBuffer.myBuffer->Unmap(myMapMode, 0u, mySize);
    myMappedData = nullptr;
    mySize = 0u;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  MappedTempTextureBuffer::MappedTempTextureBuffer(DynamicArray<TextureSubLayout> someLayouts, const TempBufferResource& aResource, GpuResourceMapMode aMapMode, uint64 aSize)
    : MappedTempBuffer(aResource, aMapMode, aSize)
    , myLayouts(std::move(someLayouts))
  {
  }
//---------------------------------------------------------------------------//
}