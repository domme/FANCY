#ifndef INCLUDE_MATHUTIL_H
#define INCLUDE_MATHUTIL_H

#include "FancyCorePrerequisites.h"

namespace Fancy {

class MathUtil {
public:
//---------------------------------------------------------------------------//
  /// Computes a hash for a given value-type and combines it into a seed.
  static void hash_combine(uint& uCombinedSeed, const uint value) 
  {
    std::hash<uint> hash_computer;
    uCombinedSeed ^= hash_computer(value * 2654435761) + 0x9e3779b9 + 
                     (uCombinedSeed << 6) + (uCombinedSeed >> 2);
  }
//-----------------------------------------------------------------------//
  static uint hashFromString(const String& szString)
  {
    if (szString.empty()) {
      return 0x0;
    }

    std::hash<std::string> hasher;
    return hasher(szString);
  }
//---------------------------------------------------------------------------//
  /// Compute a hash value based on the object's memory
  template<class T>
  static uint hashFromGeneric(const T& _val)
  {
    const size_t sizeBytes = sizeof(T);
    const uint numDwords = sizeBytes / sizeof(uint64);
    const uint numBytesRemaining = sizeBytes - numDwords * sizeof(uint64);
    const T* pMem = &_val;

    uint hash = 0x0;
    for (uint i = 0u; i < numDwords; ++i)
    {
      hash_combine(hash, reinterpret_cast<const uint64*>(pMem)[i]);
    }
    const T* byteStartBlock = pMem + numDwords * sizeof(uint64);
    for (uint i = 0u; i < numBytesRemaining; ++i)
    {
      hash_combine(hash, reinterpret_cast<const uint8*>(byteStartBlock)[i]);
    }

    return hash;
  }
//---------------------------------------------------------------------------//
  static constexpr uint Align(uint aVal, uint anAlignment)
  {
    return anAlignment == 0u ? aVal : ((aVal + (anAlignment - 1u)) & (~(anAlignment - 1u)));
  }

  static glm::mat4 perspectiveFov(float const & fov,
                           float const & width,
                           float const & height,
                           float const & zNear,
                           float const & zFar)
  {

    
    const float fovY_rad = glm::radians(fov);

    const float aspect = height / width;
    const float xScale = 1.0f / glm::tan(0.0f * fovY_rad);
    const float yScale = xScale * aspect;

    float Q1 = zFar / (zNear - zFar);
    float Q2 = zNear * Q1;

    glm::float4x4 matResult(
      xScale, 0.0f, 0.0f, 0.0f,
      0.0f, yScale, 0.0f, 0.0f,
      0.0f, 0.0f, Q1, -1.0f,
      0.0f, 0.0f, Q2, 0.0f);

    return glm::transpose(matResult);

    
    /*
    float rad = glm::radians(fov);

    float h = glm::cos(float(0.5) * rad) / glm::sin(float(0.5) * rad);
    float w = h * height / width; ///todo max(width , Height) / min(width , Height)?

    glm::mat4 Result(float(0));
    Result[0][0] = w;
    Result[1][1] = h;

#if defined (RENDERER_DX12)
    Result[2][2] = zFar / (zFar - zNear);
    Result[2][3] = 1.0f;
    Result[3][2] = -(zFar * zNear) / (zFar - zNear);
#else
    
#endif
    return Result;

    */
  }

//---------------------------------------------------------------------------//
private:
  MathUtil();
  ~MathUtil();
};

} // end of namespace Fancy

#endif  // INCLUDE_MATHUTIL_H


