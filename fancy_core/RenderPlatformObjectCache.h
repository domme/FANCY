#pragma once

#include "EASTL/hash_map.h"

namespace Fancy
{
  template<class T>
  class RenderPlatformObjectCache
  {
  public:
    virtual ~RenderPlatformObjectCache() = default;
    virtual void Clear() = 0;

  protected:
    std::mutex myCacheMutex;
    eastl::hash_map<uint64, T> myCache;
  };
}
