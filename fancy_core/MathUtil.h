#pragma once

#include "FancyCoreDefines.h"
#include "MathIncludes.h"

#include "EASTL/type_traits.h"
#include "EASTL/string.h"

namespace Fancy { namespace MathUtil {
//---------------------------------------------------------------------------//
  struct Hasher
  {
    Hasher(uint64 aSeed = 0u);
    ~Hasher();

    // Disallow copy and assignment
    // Hasher should only be used locally to produce a hash
    Hasher(const Hasher&) = delete;
    Hasher(const Hasher&&) = delete;
    Hasher& operator=(const Hasher&) = delete;

    template<class T>
    void Add(const T& aValue) { Add(&aValue, sizeof(aValue)); }

    void Add(const void* aValue, uint64 aSize);

    uint64 GetHashValue() const;

  private:
    void* myState;
  };
//---------------------------------------------------------------------------//
  // Deprecated: Use Hasher instead:
  void BeginMultiHash();
  void AddToMultiHash(const void* aValue, uint64 aSize);
  template<class T>
  void AddToMultiHash(const T& aValue) { AddToMultiHash(&aValue, sizeof(T)); }
  uint64 EndMultiHash();
  ///////////////////////////////////////////////////////
//---------------------------------------------------------------------------//
  uint64 ByteHash(const void* aValue, uint64 aSize);
//---------------------------------------------------------------------------//
  template<class T>
  inline size_t ByteHash(const T& aValue)
  {
    return ByteHash(&aValue, sizeof(T));
  }
//---------------------------------------------------------------------------//
  template<class T>
  inline size_t Hash(const T& aValue)
  {
    return eastl::hash<T>{}(aValue);
  }
//---------------------------------------------------------------------------//
  template<>
  inline size_t Hash<const char*>(const char* const& aValue)
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
  constexpr uint64 Align(uint64 aVal, uint64 anAlignment)
  {
    return (aVal + (anAlignment - 1u)) & (~(anAlignment - 1u));
  }
//---------------------------------------------------------------------------//
  constexpr bool IsAligned(uint64 aVal, uint64 anAlignment)
  {
    return (aVal & (anAlignment - 1u)) == 0u;
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