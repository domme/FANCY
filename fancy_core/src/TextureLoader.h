#ifndef INCLUDE_TEXTURELOADER_H
#define INCLUDE_TEXTURELOADER_H

#include "FancyCorePrerequisites.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  struct TextureLoadInfo
  {
    uint32 bitsPerPixel;
    uint32 numChannels;
    uint32 width;
    uint32 height;
  };
//---------------------------------------------------------------------------//
  class TextureLoader
  {
  public:
    static bool loadTexture(const std::string& _szPathAbs, std::vector<uint8>& _vOutBytes, TextureLoadInfo& _outTexLoadInfo);
  };
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO

#endif INCLUDE_TEXTURELOADER_H