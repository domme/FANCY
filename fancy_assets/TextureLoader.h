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
  class TextureLoader
  {
  public:
    static SharedPtr<Texture> CreateTexture(const TextureDesc& aDesc);

  private:
    static std::map<uint64, SharedPtr<Texture>> ourTextureCache;
    static bool loadTexture(const std::string& _szPathAbs, std::vector<uint8>& _vOutBytes, TextureLoadInfo& _outTexLoadInfo);
  };
//---------------------------------------------------------------------------//
}
