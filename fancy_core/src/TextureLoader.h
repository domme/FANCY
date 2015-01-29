#ifndef INCLUDE_TEXTURELOADER_H
#define INCLUDE_TEXTURELOADER_H

#include "FancyCorePrerequisites.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  class TextureLoader
  {
  public:
    static bool loadTexture(const std::string& _szPathAbs, std::vector<uint8>& _vOutBytes);
  };
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO

#endif INCLUDE_TEXTURELOADER_H