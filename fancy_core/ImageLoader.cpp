#include "fancy_core_precompile.h"
#include "ImageLoader.h"

#include "Log.h"
#include "MathIncludes.h"

#define STBI_FAILURE_USERMSG  // Slightly more user-friendly error-messages
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

using namespace Fancy;
//---------------------------------------------------------------------------//
  bool ImageLoader::Load(const char* aPathAbs, Image& anImageOut)
  {
    FILE* file = stbi__fopen(aPathAbs, "rb");
    if (file == nullptr)
    {
      LOG_ERROR("Unable to open image-file %s for reading", aPathAbs);
      return false;
    }
    
    glm::ivec2 size;
    int numChannels;
    if (!stbi_info_from_file(file, &size.x, &size.y, &numChannels))
    {
      LOG_ERROR("Unable to optain image-infos from file %s. Reason: %s", aPathAbs, stbi_failure_reason());
      fclose(file);
      return false;
    }

    const int forcedNumChannels = numChannels == 3 ? 4 : 0;  // Keep the original channels for everything else than RGB. In this case, always use RGBA
    std::unique_ptr<uint8, Image::Malloc_deleter> imageData(stbi_load_from_file(file, &size.x, &size.y, &numChannels, forcedNumChannels));
    if (!imageData)
    {
      LOG_ERROR("Unable to decode image file %s. Reason: %s", aPathAbs, stbi_failure_reason());
      fclose(file);
      return false;
    }

    anImageOut.myData = std::move(imageData);
    anImageOut.mySize = size;
    anImageOut.myBitsPerChannel = 8; // TODO: Add support for HDR-images
    anImageOut.myNumChannels = forcedNumChannels != 0 ? forcedNumChannels: numChannels;
    anImageOut.myByteSize = (size.x * size.y * anImageOut.myNumChannels * anImageOut.myBitsPerChannel) / 8;
    return true;
  }
//---------------------------------------------------------------------------//