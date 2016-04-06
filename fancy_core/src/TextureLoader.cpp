#include "TextureLoader.h"

#include "PathService.h"

#include "lodepng.h"
#include "Log.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  namespace Internal {
    bool loadPNG(const std::string& _szPathAbs, std::vector<uint8>& _vOutBytes, TextureLoadInfo& _outTexLoadInfo);
  }
//---------------------------------------------------------------------------//
  bool Internal::loadPNG(const std::string& _szPathAbs, std::vector<uint8>& _vOutBytes, TextureLoadInfo& _outTexLoadInfo)
  {
    std::vector<uint8> vEncodedFileBytes;
    uint32 uWidth, uHeight;
    lodepng::State state;
    state.decoder.color_convert = 0;  // Don't convert RGBA->RGB
    
    lodepng::load_file(vEncodedFileBytes, _szPathAbs);
    ASSERT(vEncodedFileBytes.size() > 0u, "Error loading image file %", _szPathAbs);

    uint32 uErrorCode = lodepng::decode(_vOutBytes, uWidth, uHeight, state, vEncodedFileBytes);
    
    if (uErrorCode != 0u)
    {
      LOG_ERROR("Failed decoding .png image % \n ErrorMessage: %", _szPathAbs, lodepng_error_text(uErrorCode) );
      return false;
    }

    const LodePNGColorMode& color = state.info_png.color;
    
    _outTexLoadInfo.width = uWidth;
    _outTexLoadInfo.height = uHeight;
    _outTexLoadInfo.bitsPerPixel = lodepng_get_bpp(&color);
    _outTexLoadInfo.numChannels = lodepng_get_channels(&color);

    return true;
  }
//---------------------------------------------------------------------------//
  bool TextureLoader::loadTexture(const std::string& _szPathAbs, std::vector<uint8>& _vOutBytes, TextureLoadInfo& _outTexLoadInfo)
  {
    std::string szFileType = PathService::getFileExtension(_szPathAbs);

    if (szFileType == "PNG" || szFileType == "png")
    {
      return Internal::loadPNG(_szPathAbs, _vOutBytes, _outTexLoadInfo);
    }

    ASSERT(false, "Missing implementation to load textures of filetype %", szFileType);
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO