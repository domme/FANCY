#pragma once

#include "FancyCorePrerequisites.h"

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
  class TextureLoader
  {
  public:
    static bool loadTexture(const std::string& _szPathAbs, std::vector<uint8>& _vOutBytes, TextureLoadInfo& _outTexLoadInfo);
  };
//---------------------------------------------------------------------------//
}
