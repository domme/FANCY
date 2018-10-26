#include "TextureLoader.h"
#include "lodepng/lodepng.h"

#include <fancy_core/PathService.h>
#include <fancy_core/Log.h>

using namespace Fancy;

//---------------------------------------------------------------------------//
  namespace Private_TextureLoader {
    bool Load_PNG(const char* _szPathAbs, std::vector<uint8>& _vOutBytes, TextureLoadInfo& _outTexLoadInfo)
    {
      std::vector<uint8> vEncodedFileBytes;
      uint uWidth, uHeight;
      lodepng::State state;
      state.decoder.color_convert = 0;  // Don't convert RGBA->RGB

      lodepng::load_file(vEncodedFileBytes, _szPathAbs);
      ASSERT(vEncodedFileBytes.size() > 0u, "Error loading image file %", _szPathAbs);

      uint uErrorCode = lodepng::decode(_vOutBytes, uWidth, uHeight, state, vEncodedFileBytes);

      if (uErrorCode != 0u)
      {
        LOG_ERROR("Failed decoding .png image % \n ErrorMessage: %", _szPathAbs, lodepng_error_text(uErrorCode));
        return false;
      }

      if (state.info_png.color.colortype != LodePNGColorType::LCT_RGBA)
      {
        std::vector<uint8> tempBuf;

        tempBuf.resize(uWidth * uHeight * 8 * 4);
      }

      const LodePNGColorMode& color = state.info_png.color;



      _outTexLoadInfo.width = uWidth;
      _outTexLoadInfo.height = uHeight;
      _outTexLoadInfo.bitsPerPixel = lodepng_get_bpp(&color);
      _outTexLoadInfo.numChannels = lodepng_get_channels(&color);

      return true;
    }
  }
//---------------------------------------------------------------------------//
  bool TextureLoader::Load(const char* aPath, DynamicArray<uint8>& _vOutBytes, TextureLoadInfo& _outTexLoadInfo)
  {
    const String& extension = Path::GetFileExtension(aPath);

    if (extension == "PNG" || extension == "png")
    {
      return Private_TextureLoader::Load_PNG(aPath, _vOutBytes, _outTexLoadInfo);
    }

    ASSERT(false, "Missing implementation to load textures of filetype %", extension);
    return false;
  }
//---------------------------------------------------------------------------//
