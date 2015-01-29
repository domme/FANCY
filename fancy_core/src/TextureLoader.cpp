#include "TextureLoader.h"

#include "PathService.h"

#include "lodepng.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  namespace Internal {
    bool loadPNG(const std::string& _szPathAbs, std::vector<uint8>& _vOutBytes);
  }
//---------------------------------------------------------------------------//
  bool Internal::loadPNG(const std::string& _szPathAbs, std::vector<uint8>& _vOutBytes)
  {

  }
//---------------------------------------------------------------------------//
  bool TextureLoader::loadTexture(const std::string& _szPathAbs, std::vector<uint8>& _vOutBytes)
  {
    std::string szFileType = PathService::getFileType(_szPathAbs);

    if (szFileType == "PNG" || szFileType == "png")
    {
      return Internal::loadPNG(_szPathAbs, _vOutBytes);
    }

    ASSERT_M(false, std::string("Missing implementation to load textures of filetype ") + szFileType);
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO