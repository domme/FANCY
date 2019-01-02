#pragma once

#include "FancyCoreDefines.h"
#include "MathUtil.h"

namespace Fancy {
//---------------------------------------------------------------------------// 
  template<class T, uint AlignmentBytes = 1u>  // Default: No alignment at all
  struct AlignedStorage
  {
    uint8 myBytes[MathUtil::Align(sizeof(T), AlignmentBytes)];
  };
//---------------------------------------------------------------------------//
}  // end of namespace Fancy
