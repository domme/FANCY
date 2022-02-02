#pragma once

#include "Common/FancyCoreDefines.h"
#include "Common/MathIncludes.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct Image
  {
    struct Malloc_deleter
    {
      void operator()(uint8* aPtr) const { free(aPtr); }
    };

    std::unique_ptr<uint8, Malloc_deleter> myData;
    uint myBitsPerChannel = 0;
    uint myNumChannels = 0;
    glm::ivec2 mySize = glm::ivec2(0,0);
    uint myByteSize = 0;
  };
//---------------------------------------------------------------------------//
  struct ImageLoader
  {
    static bool Load(const char* aPathAbs, Image& anImageOut);
  };
//---------------------------------------------------------------------------//
}
