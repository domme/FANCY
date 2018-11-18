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

    auto listIt = myAvailableTextures.find(key);
    if (listIt != myAvailableTextures.end() && !listIt->second.empty())
    {
      std::list<TextureResource*>& availableList = listIt->second;
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
    res.Create(someProps, aName);
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

    auto listIt = myAvailableTextures.find(aBucketHash);
    listIt->second.push_back(&it->second);

    ASSERT(myNumOpenFrameAllocs > 0);
    --myNumOpenFrameAllocs;
  }
//---------------------------------------------------------------------------//
}

