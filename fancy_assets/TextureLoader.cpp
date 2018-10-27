#include "TextureLoader.h"
#include "lodepng/lodepng.h"

#include <fancy_core/PathService.h>
#include <fancy_core/Log.h>

using namespace Fancy;

//---------------------------------------------------------------------------//
  namespace Private_TextureLoader {
    bool Load_PNG(const char* aPathAbs, std::vector<uint8>& someTextureBytesOut, TextureLoadInfo& aLoadInfoOut)
    {
      std::vector<uint8> fileBuf;
      lodepng::load_file(fileBuf, aPathAbs);
      if (fileBuf.empty())
      {
        LOG_ERROR("Error loading image file %", aPathAbs);
        return false;
      }

      uint width, height;
      lodepng::State state;
      uint error = lodepng_inspect(&width, &height, &state, fileBuf.data(), fileBuf.size());
      if (error != 0u)
      {
        LOG_ERROR("Failed reading png header for image % \n ErrorMessage: %", aPathAbs, lodepng_error_text(error));
        return false;
      }

      uint numChannels = lodepng_get_channels(&state.info_png.color);
      switch(numChannels)
      {
        case 1: state.info_raw.colortype = LCT_GREY; break;
        case 2: state.info_raw.colortype = LCT_GREY_ALPHA; break;
        case 3: state.info_raw.colortype = LCT_RGB; break;
        default: 
          state.info_raw.colortype = LCT_RGBA; 
          numChannels = 4; 
          break;
      }
        
      error = lodepng::decode(someTextureBytesOut, width, height, state, fileBuf);
      if (error != 0u)
      {
        LOG_ERROR("Failed decoding .png image % \n ErrorMessage: %", aPathAbs, lodepng_error_text(error));
        return false;
      }

      aLoadInfoOut.width = width;
      aLoadInfoOut.height = height;
      aLoadInfoOut.numChannels = numChannels;
      aLoadInfoOut.bitsPerChannel = state.info_raw.bitdepth;

      return true;
    }
  }
//---------------------------------------------------------------------------//
  bool TextureLoader::Load(const char* aPath, DynamicArray<uint8>& someTextureBytesOut, TextureLoadInfo& aLoadInfoOut)
  {
    const String& extension = Path::GetFileExtension(aPath);

    if (extension == "PNG" || extension == "png")
    {
      return Private_TextureLoader::Load_PNG(aPath, someTextureBytesOut, aLoadInfoOut);
    }

    ASSERT(false, "Missing implementation to load textures of filetype %", extension);
    return false;
  }
//---------------------------------------------------------------------------//
