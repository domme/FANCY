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
private:
  MathUtil();
  ~MathUtil();
};

} // end of namespace Fancy

#endif  // INCLUDE_MATHUTIL_H


