#pragma once
#include <unordered_map>
#include "TextureResource.h"

namespace Fancy 
{
//---------------------------------------------------------------------------//
  class TempResourcePool;
  struct TempResourceKeepAlive
  {
    TempResourceKeepAlive(TempResourcePool* aPool, void* aResource, uint64 aBucketHash)
      : myPool(aPool), myResource(aResource), myBucketHash(aBucketHash) {}

    ~TempResourceKeepAlive();

  private:
    TempResourcePool* myPool;
    void* myResource;
    uint64 myBucketHash;
  };
//---------------------------------------------------------------------------//
  struct TempTextureResource
  {
    friend class TempResourcePool;

    TempTextureResource() = default;

    TempTextureResource(const TempTextureResource& anOther)
      : myTexture(anOther.myTexture)
      , myReadView(anOther.myReadView)
      , myWriteView(anOther.myWriteView)
      , myRenderTargetView(anOther.myRenderTargetView)
      , myKeepAlive(anOther.myKeepAlive)
    {

    }

    TempTextureResource& operator=(const TempTextureResource& anOther)
    {
      myTexture = anOther.myTexture;
      myReadView = anOther.myReadView;
      myWriteView = anOther.myWriteView;
      myRenderTargetView = anOther.myRenderTargetView;
      myKeepAlive = anOther.myKeepAlive;
      return *this;
    }

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
  class TempResourcePool
  {
    friend class TempResourceKeepAlive;

  public:
    enum Flags
    {
      FORCE_SIZE,
      
      NUM,
    };
    
    TempResourcePool() = default;
    ~TempResourcePool();
    
    void EndFrame();
    TempTextureResource AllocateTexture(const TextureResourceProperties& someProps, uint someFlags, const char* aName = nullptr);

  private:
    void FreeResource(void* aResource, uint64 aBucketHash);

    std::unordered_map<uint64, std::list<TextureResource*>> myAvailableTextures;
    std::unordered_map<Texture*, TextureResource> myTexturePool;
    uint myNumOpenFrameAllocs = 0u;

    // TODO: Add GpuBuffers...
  };
//---------------------------------------------------------------------------//
}


