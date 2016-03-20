#pragma once

#include "FancyCorePrerequisites.h"
#include "SmallObjectAllocator.h"

namespace Fancy {
//---------------------------------------------------------------------------// 
  template<class T>
  class ObjectPool : public SmallObjectAllocator<T>
  {
  public:
    ObjectPool(uint32 anExpectedObjectNum = 256u) 
      : SmallObjectAllocator<T>(anExpectedObjectNum / 64u)
    {}
  };
//---------------------------------------------------------------------------//
}

