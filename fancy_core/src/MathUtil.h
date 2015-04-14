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

    std::hash<String> hasher;
    return hasher(szString);
  }
//---------------------------------------------------------------------------//
  /// Compute a hash value based on the object's memory
  template<class T>
  static uint hashFromGeneric(const T& _val)
  {
    const uint32 sizeBytes = sizeof(T);
    const T* pMem = &_val;

    uint hash = 0x0;
    for (uint32 i = 0u; i < sizeBytes; ++i)
    {
      hash_combine(hash, reinterpret_cast<const uint8*>(pMem)[i]);
    }

    return hash;
  }
//---------------------------------------------------------------------------//
  static uint32 floatToUintBits(float _fVal)
  {
    STATIC_ASSERT(sizeof(uint32) == sizeof(float), "Sizes don't match (unsupported platform?)");

    uint32 floatBits = 0x0;
    uint8* const pFloatBits = reinterpret_cast<uint8*>(&floatBits);
    std::copy(pFloatBits, pFloatBits + sizeof(float), reinterpret_cast<uint8*>(&_fVal));

    return floatBits;
  }
//---------------------------------------------------------------------------//
private:
  MathUtil();
  ~MathUtil();
};

} // end of namespace Fancy

#endif  // INCLUDE_MATHUTIL_H


