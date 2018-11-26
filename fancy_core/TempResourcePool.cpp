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
      uint64 hash;
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
  }
//---------------------------------------------------------------------------//
  TempResourceKeepAlive::~TempResourceKeepAlive()
  {
    myPool->FreeResource(myResource, myBucketHash);
  }
//---------------------------------------------------------------------------//
  TempResourcePool::~TempResourcePool()
  {
    ASSERT(myNumOpenFrameAllocs == 0, "% open temp resource allocs when destroying the temp resource pool", myNumOpenFrameAllocs);
  }
//---------------------------------------------------------------------------//
  void TempResourcePool::EndFrame()
  {
    ASSERT(myNumOpenFrameAllocs == 0, "% open temp resource allocs accross frame boundary", myNumOpenFrameAllocs);
  }
//---------------------------------------------------------------------------//
  TempTextureResource TempResourcePool::AllocateTexture(const TextureResourceProperties& someProps, uint someFlags, const char* aName)
  {
    const uint64 key = Priv_TempResourcePool::GetHash(someProps, someFlags);

    std::list<TextureResource*>& availableList = myAvailableTextureBuckets[key];
    if (!availableList.empty())
    {
      auto it = availableList.begin();
      for (; it != availableList.end(); ++it)
      {
        const TextureProperties& texProps = (*it)->myTexture->GetProperties();

        if (someFlags & FORCE_SIZE)
        {
          if (texProps.myWidth == someProps.myTextureProperties.myWidth && texProps.myHeight == someProps.myTextureProperties.myHeight && texProps.myDepthOrArraySize == someProps.myTextureProperties.myDepthOrArraySize)
            break;
        }
        else
        {
          if (texProps.myWidth >= someProps.myTextureProperties.myWidth && texProps.myHeight >= someProps.myTextureProperties.myHeight && texProps.myDepthOrArraySize >= someProps.myTextureProperties.myDepthOrArraySize)
            break;
        }
      }

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
        ++myNumOpenFrameAllocs;
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
    ++myNumOpenFrameAllocs;
    return returnRes;
  }
//---------------------------------------------------------------------------//
  void TempResourcePool::FreeResource(void* aResource, uint64 aBucketHash)
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

    ASSERT(myNumOpenFrameAllocs > 0);
    --myNumOpenFrameAllocs;
  }
//---------------------------------------------------------------------------//
}

