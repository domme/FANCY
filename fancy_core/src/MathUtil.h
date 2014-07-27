#ifndef INCLUDE_MATHUTIL_H
#define INCLUDE_MATHUTIL_H

#include "FancyCorePrerequisites.h"

namespace FANCY { namespace Core {

class MathUtil {
public:

  /// Computes a hash for a given value-type and combines it into a seed.
  static void hash_combine(size_t& uCombinedSeed, const size_t& value) 
  {
    std::hash<size_t> hash_computer;
    uCombinedSeed ^= hash_computer(value * 2654435761) + 0x9e3779b9 + 
                     (uCombinedSeed << 6) + (uCombinedSeed >> 2);
  }
//-----------------------------------------------------------------------//
  

private:
  MathUtil();
  ~MathUtil();
};

} // end of namespace Core
} // end of namespace FANCY

#endif  // INCLUDE_MATHUTIL_H


