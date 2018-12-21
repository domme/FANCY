#include "stdafx.h"
#include "TempResourcePool.h"
#include "MathUtil.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  namespace Priv_TempResourcePool
  {
    uint64 GetHash(const TextureResourceProperties& someProps, uint someFlags)
    {
      uint64 hash = 0u;
      MathUtil::hash_combine(hash, static_cast<uint>(someProps.myTextureProperties.myDimension));
      MathUtil::hash_combine(hash, static_cast<uint>(someProps.myTextureProperties.eFormat));
      MathUtil::hash_combine(hash, someProps.myTextureProperties.IsArray() ? 1 : 0);
      MathUtil::hash_combine(hash, someProps.myTextureProperties.myNumMipLevels ? 1 : 0);
      MathUtil::hash_combine(hash, static_cast<uint>(someProps.myTextureProperties.GetDepthSize()));
      MathUtil::hash_combine(hash, static_cast<uint>(someProps.myTextureProperties.myAccessType));
      MathUtil::hash_combine(hash, someProps.myIsRenderTarget ? 1 : 0);
      MathUtil::hash_combine(hash, someProps.myIsTexture ? 1 : 0);
      MathUtil::hash_combine(hash, someProps.myIsShaderWritable ? 1 : 0);
      return hash;
    }

    uint64 GetHash(const GpuBufferResourceProperties& someProps, uint someFlags)
    {
      uint64 hash = 0u;
      MathUtil::hash_combine(hash, static_cast<uint>(someProps.myFormat));
      MathUtil::hash_combine(hash, someProps.myStructureSize);
      MathUtil::hash_combine(hash, someProps.myIsStructured ? 1 : 0);
      MathUtil::hash_combine(hash, someProps.myIsRaw ? 1 : 0);
      MathUtil::hash_combine(hash, someProps.myIsShaderResource ? 1 : 0);
      MathUtil::hash_combine(hash, someProps.myIsShaderWritable ? 1 : 0);
      MathUtil::hash_combine(hash, someProps.myBufferProperties.myCpuAccess);
      MathUtil::hash_combine(hash, someProps.myBufferProperties.myUsage);
      return hash;
    }
  }
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

//---------------------------------------------------------------------------//
  TempResourcePool::~TempResourcePool()
  {
    ASSERT(myNumOpenBufferAllocs == 0, "% open temp buffer allocs when destroying the temp resource pool", myNumOpenBufferAllocs);
    ASSERT(myNumOpenTextureAllocs == 0, "% open temp texture allocs when destroying the temp resource pool", myNumOpenTextureAllocs);
  }
//---------------------------------------------------------------------------//
  void TempResourcePool::Reset()
  {
    ASSERT(myNumOpenBufferAllocs == 0, "% open temp buffer allocs accross frame boundary", myNumOpenBufferAllocs);
    ASSERT(myNumOpenTextureAllocs == 0, "% open temp texture allocs accross frame boundary", myNumOpenTextureAllocs);
  }
//---------------------------------------------------------------------------//
  TempTextureResource TempResourcePool::AllocateTexture(const TextureResourceProperties& someProps, uint someFlags, const char* aName)
  {
    const uint64 key = Priv_TempResourcePool::GetHash(someProps, someFlags);

    std::list<TextureResource*>& availableList = myAvailableTextureBuckets[key];
    if (!availableList.empty())
    {
      auto it = std::find_if(availableList.begin(), availableList.end(), [&](const TextureResource* aResource) 
      {
        const TextureProperties& texProps = aResource->myTexture->GetProperties();

        if (someFlags & FORCE_SIZE)
          return texProps.myWidth == someProps.myTextureProperties.myWidth && texProps.myHeight == someProps.myTextureProperties.myHeight && texProps.myDepthOrArraySize == someProps.myTextureProperties.myDepthOrArraySize;

        return texProps.myWidth >= someProps.myTextureProperties.myWidth && texProps.myHeight >= someProps.myTextureProperties.myHeight && texProps.myDepthOrArraySize >= someProps.myTextureProperties.myDepthOrArraySize;
      });

      if (it != availableList.end())
      {
        TextureResource* res = (*it);
        availableList.erase(it);

        TempTextureResource returnRes;
        returnRes.myTexture = res->myTexture.get();
        returnRes.myReadView = res->myReadView.get();
        returnRes.myWriteView = res->myWriteView.get();
        returnRes.myRenderTargetView = res->myRenderTargetView.get();
        returnRes.myKeepAlive.reset(new TempResourceKeepAlive(this, returnRes.myTexture, key));
        ++myNumOpenTextureAllocs;
        return returnRes;
      }
    }

    // Create resource
    TextureResource res;
    res.Update(someProps, aName);
    myTexturePool[res.myTexture.get()] = res;

    TempTextureResource returnRes;
    returnRes.myTexture = res.myTexture.get();
    returnRes.myReadView = res.myReadView.get();
    returnRes.myWriteView = res.myWriteView.get();
    returnRes.myRenderTargetView = res.myRenderTargetView.get();
    returnRes.myKeepAlive.reset(new TempResourceKeepAlive(this, returnRes.myTexture, key));
    ++myNumOpenTextureAllocs;
    return returnRes;
  }
//---------------------------------------------------------------------------//
  TempBufferResource TempResourcePool::AllocateBuffer(const GpuBufferResourceProperties& someProps, uint someFlags, const char* aName)
  {
    const uint64 desiredSize = someProps.myBufferProperties.myNumElements * someProps.myBufferProperties.myElementSizeBytes;

    const uint64 key = Priv_TempResourcePool::GetHash(someProps, someFlags);
    std::list<GpuBufferResource*>& availableList = myAvailableBufferBuckets[key];
    if (!availableList.empty())
    {
      auto it = std::find_if(availableList.begin(), availableList.end(), [desiredSize, someFlags](const GpuBufferResource* aResource)
      {
        const uint64 currSize = aResource->myBuffer->GetByteSize();
        return (someFlags & FORCE_SIZE) > 0 ? currSize == desiredSize : currSize >= desiredSize;
      });

      if (it != availableList.end())
      {
        GpuBufferResource* res = (*it);
        availableList.erase(it);

        TempBufferResource returnRes;
        returnRes.myBuffer = res->myBuffer.get();
        returnRes.myReadView = res->myReadView.get();
        returnRes.myWriteView = res->myWriteView.get();
        returnRes.myKeepAlive.reset(new TempResourceKeepAlive(this, returnRes.myBuffer, key));
        ++myNumOpenBufferAllocs;
        return returnRes;
      }
    }

    // Create resource
    GpuBufferResource res;
    res.Update(someProps, aName);
    myBufferPool[res.myBuffer.get()] = res;

    TempBufferResource returnRes;
    returnRes.myBuffer = res.myBuffer.get();
    returnRes.myReadView = res.myReadView.get();
    returnRes.myWriteView = res.myWriteView.get();
    returnRes.myKeepAlive.reset(new TempResourceKeepAlive(this, returnRes.myBuffer, key));
    ++myNumOpenBufferAllocs;
    return returnRes;
  }
//---------------------------------------------------------------------------//
  void TempResourcePool::FreeResource(void* aResource, uint64 aBucketHash)
  {
    GpuResource* resource = static_cast<GpuResource*>(aResource);
    if (resource->myCategory == GpuResourceCategory::BUFFER)
    {
      auto it = myBufferPool.find(static_cast<GpuBuffer*>(aResource));
      ASSERT(it != myBufferPool.end());
      auto listIt = myAvailableBufferBuckets.find(aBucketHash);
      ASSERT(listIt != myAvailableBufferBuckets.end());

      GpuBufferResource* res = &it->second;
#if FANCY_RENDERER_HEAVY_VALIDATION
      ASSERT(std::find(listIt->second.begin(), listIt->second.end(), res) == listIt->second.end());
#endif
      listIt->second.push_back(res);

      ASSERT(myNumOpenBufferAllocs > 0);
      --myNumOpenBufferAllocs;
    }
    else
    {
      auto it = myTexturePool.find(static_cast<Texture*>(aResource));
      ASSERT(it != myTexturePool.end());
      auto listIt = myAvailableTextureBuckets.find(aBucketHash);
      ASSERT(listIt != myAvailableTextureBuckets.end());

      TextureResource* res = &it->second;
#if FANCY_RENDERER_HEAVY_VALIDATION
      ASSERT(std::find(listIt->second.begin(), listIt->second.end(), res) == listIt->second.end());
#endif
      listIt->second.push_back(res);

      ASSERT(myNumOpenTextureAllocs > 0);
      --myNumOpenTextureAllocs;
    }
  }
//---------------------------------------------------------------------------//
}

