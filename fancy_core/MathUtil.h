#pragma once

#include "FancyCoreDefines.h"
#include "MathIncludes.h"

#include <type_traits>  // std::hash

namespace Fancy { namespace MathUtil {
//---------------------------------------------------------------------------//
  inline size_t ByteHash(const uint8* aValue, uint64 aSize)
  {
    uint64 hash = 0x0;
    std::hash<uint> hasher;
    for (uint i = 0u; i < aSize; ++i)
      hash ^= hasher(static_cast<uint>(aValue[i]) * 2654435761u) + 0x9e3779b9 + (hash << 6) + (hash >> 2);

    return hash;
  }
//---------------------------------------------------------------------------//
  template<class T>
  inline size_t ByteHash(const T& aValue)
  {
    const size_t sizeBytes = sizeof(T);
    const uint8* byteBlock = reinterpret_cast<const uint8*>(&aValue);
    return ByteHash(byteBlock, sizeBytes);
  }
//---------------------------------------------------------------------------//
  template<class T>
  inline size_t Hash(const T& aValue)
  {
    return std::hash<T>{}(aValue);
  }
//---------------------------------------------------------------------------//
  template<>
  inline size_t Hash<const char*&>(const char*& aValue)
  {
    return ByteHash(reinterpret_cast<const uint8*>(aValue), strlen(aValue));
  }
//---------------------------------------------------------------------------//
  template<class T>
  inline void hash_combine(uint64& uCombinedSeed, const T& value)
  {
    uCombinedSeed ^= Hash<T>(value) * 2654435761 + 0x9e3779b9 +
                     (uCombinedSeed << 6) + (uCombinedSeed >> 2);
  }
//---------------------------------------------------------------------------//
  inline constexpr uint64 Align(uint64 aVal, uint64 anAlignment)
  {
    return anAlignment == 0u ? aVal : ((aVal + (anAlignment - 1u)) & (~(anAlignment - 1u)));
  }
//---------------------------------------------------------------------------//
  inline glm::mat4 perspectiveFov(float const & fov,
                           float const & width,
                           float const & height,
                           float const & zNear,
                           float const & zFar)
  {
    const float fovY_rad = glm::radians(fov);

    const float aspect = width / height;
    const float xScale = 1.0f / glm::tan(0.5f * fovY_rad);
    const float yScale = xScale * aspect;

    float A = zFar / (zFar - zNear);
    float B = -zNear * A;

    glm::float4x4 matResult(0);
    matResult[0] = glm::float4(xScale, 0.0f, 0.0f, 0.0f);
    matResult[1] = glm::float4(0.0f, yScale, 0.0f, 0.0f);
    matResult[2] = glm::float4(0.0f, 0.0f, A, 1.0f);
    matResult[3] = glm::float4(0.0f, 0.0f, B, 0.0f);

    return matResult;
  }
//---------------------------------------------------------------------------//
  inline uint FloatBitsToUint(float aValue)
  {
    union UnionType
    {
      float fVal;
      uint uVal;
    };

    UnionType helper;
    helper.fVal = aValue;
    
    return helper.uVal;
  }
//---------------------------------------------------------------------------//
  template<typename T>
  inline constexpr T Log2(T n)
  {
    return ((n < 2) ? 1 : 1 + Log2(n / 2));
  }
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::MathUtil