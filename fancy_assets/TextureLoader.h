#pragma once

#include <fancy_core/FancyCorePrerequisites.h>

namespace Fancy {
//---------------------------------------------------------------------------//
  struct TextureLoadInfo
  {
    uint bitsPerPixel;
    uint numChannels;
    uint width;
    uint height;
  };
//---------------------------------------------------------------------------//
  struct TextureLoader
  {
    static bool Load(const char* aPath, DynamicArray<uint8>& _vOutBytes, TextureLoadInfo& _outTexLoadInfo);
  };
//---------------------------------------------------------------------------//
}
