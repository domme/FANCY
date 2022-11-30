#include "fancy_core_precompile.h"
#include "ImageLoader.h"

#include "Debug/Log.h"
#include "Common/MathIncludes.h"
#include "IO/PathService.h"

#define STBI_FAILURE_USERMSG  // Slightly more user-friendly error-messages
#define STB_IMAGE_IMPLEMENTATION
#include "dds.h"
#include "stb/stb_image.h"

using namespace Fancy;

static bool LoadImage_STB(FILE* file, const char* aPathAbs, Image& anImageOut)
{
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
  anImageOut.myNumChannels = forcedNumChannels != 0 ? forcedNumChannels : numChannels;
  anImageOut.myByteSize = (size.x * size.y * anImageOut.myNumChannels * anImageOut.myBitsPerChannel) / 8;
  return true;
}

static bool LoadImage_DDS(FILE* file, const char* aPathAbs, Image& anImageOut)
{
  uint magic = 0;
  if (fread(&magic, sizeof(magic), 1, file) != 1)
  {
    LOG_ERROR("Corrupt DDS file: Couldn't read DDS magic (file %s)", aPathAbs);
    return false;
  }

  DDS_HEADER header;
  if (fread(&header, sizeof(header), 1, file) != 1)
  {
    LOG_ERROR("Corrupt DDS file: Couldn't read DDS header (file %s)", aPathAbs);
    return false;
  }

  DDS_HEADER_DXT10 header10;
  if (header.ddspf.flags == DDS_FOURCC && header.ddspf.fourCC == DDSPF_DX10.fourCC )
  {
    if (fread(&header10, sizeof(header10), 1, file) != 1)
    {
      LOG_ERROR("Corrupt DDS file: Couldn't read DDS10 header when it should be there (file %s)", aPathAbs);
      return false;
    }
  }
}

//---------------------------------------------------------------------------//
  bool ImageLoader::Load(const char* aPathAbs, Image& anImageOut)
  {
    FILE* file = stbi__fopen(aPathAbs, "rb");
    if (file == nullptr)
    {
      LOG_ERROR("Unable to open image-file %s for reading", aPathAbs);
      return false;
    }

    eastl::string extension = Path::GetFileExtension(aPathAbs);

    if (extension.comparei("dds") == 0)
    {
      return LoadImage_DDS(file, aPathAbs, anImageOut);
    }
    else
    {
      return LoadImage_STB(file, aPathAbs, anImageOut);
    }
    
    
  }
//---------------------------------------------------------------------------//