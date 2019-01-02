#pragma once

#include <memory>

namespace Fancy
{
  template<class T>
  using SharedPtr = std::shared_ptr<T>;

  template<class T>
  using UniquePtr = std::unique_ptr<T>;
}
