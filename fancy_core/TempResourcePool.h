#pragma once

#include "GraphicsResources.h"
#include "TempResources.h"

#include <unordered_map>
#include <list>

namespace Fancy 
{
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


