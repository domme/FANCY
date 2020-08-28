#pragma once

#include "EASTL/shared_ptr.h"
#include "EASTL/unique_ptr.h"

namespace Fancy
{
  template<class T>
  using SharedPtr = eastl::shared_ptr<T>;

  template<class T>
  using UniquePtr = eastl::unique_ptr<T>;
}
