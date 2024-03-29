#pragma once

#include "Common/FancyCoreDefines.h"
#include "Common/Ptr.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  class TempResourcePool;
  class GpuBuffer;
  class GpuBufferView;
  class Texture;
  class TextureView;
//---------------------------------------------------------------------------//
  struct TempResourceKeepAlive
  {
    TempResourceKeepAlive(TempResourcePool* aPool, void* aResource, uint64 aBucketHash)
      : myPool(aPool), myResource(aResource), myBucketHash(aBucketHash) {}

    ~TempResourceKeepAlive();

  private:
    TempResourcePool * myPool;
    void* myResource;
    uint64 myBucketHash;
  };
  //---------------------------------------------------------------------------//
  struct TempBufferResource
  {
    friend class TempResourcePool;

    TempBufferResource()
      : myBuffer(nullptr)
      , myReadView(nullptr)
      , myWriteView(nullptr)
    {

    }

    TempBufferResource(const TempBufferResource& anOther)
      : myBuffer(anOther.myBuffer)
      , myReadView(anOther.myReadView)
      , myWriteView(anOther.myWriteView)
      , myKeepAlive(anOther.myKeepAlive)
    {

    }

    TempBufferResource& operator=(const TempBufferResource& anOther)
    {
      myBuffer = anOther.myBuffer;
      myReadView = anOther.myReadView;
      myWriteView = anOther.myWriteView;
      myKeepAlive = anOther.myKeepAlive;
      return *this;
    }

    TempBufferResource& operator=(TempBufferResource&& anOther) noexcept
    {
      myBuffer = anOther.myBuffer;
      myReadView = anOther.myReadView;
      myWriteView = anOther.myWriteView;
      myKeepAlive = anOther.myKeepAlive;
      return *this;
    }

    GpuBuffer* myBuffer;
    GpuBufferView* myReadView;
    GpuBufferView* myWriteView;

  protected:
    SharedPtr<TempResourceKeepAlive> myKeepAlive;
  };
  //---------------------------------------------------------------------------//
  struct TempTextureResource
  {
    friend class TempResourcePool;

    TempTextureResource()
      : myTexture(nullptr)
      , myReadView(nullptr)
      , myWriteView(nullptr)
      , myRenderTargetView(nullptr)
    {

    }

    TempTextureResource(const TempTextureResource& anOther) = default;
    TempTextureResource& operator=(const TempTextureResource& anOther) = default;

    TempTextureResource& operator=(TempTextureResource&& anOther) noexcept
    {
      myTexture = anOther.myTexture;
      myReadView = anOther.myReadView;
      myWriteView = anOther.myWriteView;
      myRenderTargetView = anOther.myRenderTargetView;
      myKeepAlive = anOther.myKeepAlive;
      return *this;
    }

    Texture* myTexture;
    TextureView* myReadView;
    TextureView* myWriteView;
    TextureView* myRenderTargetView;

  protected:
    SharedPtr<TempResourceKeepAlive> myKeepAlive;
  };
//---------------------------------------------------------------------------//
}