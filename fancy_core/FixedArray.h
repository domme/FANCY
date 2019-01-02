#pragma once

#include <array>

namespace Fancy
{
  template<class T, size_t N>
  using FixedArray = std::array<T, N>;
}
