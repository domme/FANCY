#pragma once

#include <fancy_core/FancyCorePrerequisites.h>

namespace Fancy {
//---------------------------------------------------------------------------//
  struct TextureDesc;
  class Texture;
//---------------------------------------------------------------------------//
  struct TextureLoadInfo
  {
    uint bitsPerPixel;
    uint numChannels;
    uint width;
    uint height;
  };
//---------------------------------------------------------------------------//
  namespace TextureLoader
  {
    bool Load(const std::string& _szPathAbs, std::vector<uint8>& _vOutBytes, TextureLoadInfo& _outTexLoadInfo);
  };
//---------------------------------------------------------------------------//
}
