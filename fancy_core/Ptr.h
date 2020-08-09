#pragma once

namespace Fancy
{
  template<class T>
  using SharedPtr = std::shared_ptr<T>;
  //using SharedPtr = eastl::shared_ptr<T>;

  template<class T>
  using UniquePtr = std::unique_ptr<T>;
  // using UniquePtr = eastl::unique_ptr<T>;
}
