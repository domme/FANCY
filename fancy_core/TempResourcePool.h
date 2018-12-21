#pragma once
#include <unordered_map>
#include "GraphicsResources.h"

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
  struct MappedTempBuffer
  {
    MappedTempBuffer(const TempBufferResource& aResource, GpuResourceMapMode aMapMode, uint64 aSize);
    ~MappedTempBuffer();
    void Unmap();

    TempBufferResource myTempBuffer;
    GpuResourceMapMode myMapMode = GpuResourceMapMode::WRITE;
    uint64 mySize = 0u;
    void* myMappedData = nullptr;
  };
//---------------------------------------------------------------------------//
  struct MappedTempTextureBuffer : MappedTempBuffer
  {
    MappedTempTextureBuffer(DynamicArray<TextureSubLayout> someLayouts, const TempBufferResource& aResource, GpuResourceMapMode aMapMode, uint64 aSize);
    DynamicArray<TextureSubLayout> myLayouts;
  };
//---------------------------------------------------------------------------//
  class TempResourcePool
  {
    friend struct TempResourceKeepAlive;

  public:
    enum Flags
    {
      FORCE_SIZE,
      
      NUM,
    };
    
    TempResourcePool() = default;
    ~TempResourcePool();
    
    void Reset();
    TempTextureResource AllocateTexture(const TextureResourceProperties& someProps, uint someFlags, const char* aName = nullptr);
    TempBufferResource AllocateBuffer(const GpuBufferResourceProperties& someProps, uint someFlags, const char* aName = nullptr);

  private:
    void FreeResource(void* aResource, uint64 aBucketHash);

    std::unordered_map<uint64, std::list<TextureResource*>> myAvailableTextureBuckets;
    std::unordered_map<Texture*, TextureResource> myTexturePool;
    std::unordered_map<uint64, std::list<GpuBufferResource*>> myAvailableBufferBuckets;
    std::unordered_map<GpuBuffer*, GpuBufferResource> myBufferPool;

    uint myNumOpenBufferAllocs = 0u;
    uint myNumOpenTextureAllocs = 0u;
  };
//---------------------------------------------------------------------------//
}


