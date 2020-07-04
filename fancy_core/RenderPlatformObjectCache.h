#pragma once

#include <mutex>
#include <unordered_map>

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
    std::unordered_map<uint64, T> myCache;
  };
}
