#include "fancy_core_precompile.h"
#include "ImageLoader.h"

#include "Debug/Log.h"
#include "Common/MathIncludes.h"
#include "IO/PathService.h"

#define STBI_FAILURE_USERMSG  // Slightly more user-friendly error-messages
#define STB_IMAGE_IMPLEMENTATION
#include "dds.h"
#include "Rendering/DX12/RenderCore_PlatformDX12.h"
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

  bool hasDX10Header = false;
  DDS_HEADER_DXT10 header10;
  if (header.ddspf.flags == DDS_FOURCC && header.ddspf.fourCC == DDSPF_DX10.fourCC )
  {
    hasDX10Header = true;
    if (fread(&header10, sizeof(header10), 1, file) != 1)
    {
      LOG_ERROR("Corrupt DDS file: Couldn't read DDS10 header when it should be there (file %s)", aPathAbs);
      return false;
    }
  }
  
  uint width = header.width;
  uint height = header.height;
  DataFormat format = DataFormat::UNKNOWN;

  const bool hasMips = (header.flags & DDS_HEADER_FLAGS_MIPMAP) != 0;
  const bool isVolumeTexture = (header.flags & DDS_HEADER_FLAGS_VOLUME) != 0;

  bool isCubeMap = false;
  bool isArrayTexture = false;
  GpuResourceDimension dimension = GpuResourceDimension::UNKONWN;

  if ( hasDX10Header )
  {
    const DXGI_FORMAT dxgiFormat = header10.dxgiFormat;
    format = RenderCore_PlatformDX12::ResolveDXGIFormat(dxgiFormat);

    isCubeMap = (header10.miscFlag & DDS_RESOURCE_MISC_TEXTURECUBE) != 0;
    isArrayTexture = header10.arraySize > 1;

    switch( header10.resourceDimension )
    {
      case DDS_DIMENSION_TEXTURE1D:
        dimension = isArrayTexture ? GpuResourceDimension::TEXTURE_1D_ARRAY : GpuResourceDimension::TEXTURE_1D;
        break;
      case DDS_DIMENSION_TEXTURE2D:
        if (isCubeMap)
          dimension = isArrayTexture ? GpuResourceDimension::TEXTURE_CUBE_ARRAY : GpuResourceDimension::TEXTURE_CUBE;
        else
          dimension = isArrayTexture ? GpuResourceDimension::TEXTURE_2D_ARRAY : GpuResourceDimension::TEXTURE_2D;
        break;
      case DDS_DIMENSION_TEXTURE3D:
        dimension = GpuResourceDimension::TEXTURE_3D;
        break;
      default: ASSERT(false);
    }
  }
  else
  {
    const bool isCompressed = (header.ddspf.flags & DDS_FOURCC) != 0;

    if (isCompressed)
    {
      if (header.ddspf.fourCC == DDSPF_DXT1.fourCC)
        format = DataFormat::BC1;
      else if (header.ddspf.fourCC == DDSPF_DXT5.fourCC)
        format = DataFormat::BC3;
      else if (header.ddspf.fourCC == DDSPF_BC4_UNORM.fourCC)
        format = DataFormat::BC4;
      else if (header.ddspf.fourCC == DDSPF_BC4_SNORM.fourCC)
        format = DataFormat::BC4_SNORM;
      else if (header.ddspf.fourCC == DDSPF_BC5_UNORM.fourCC)
        format = DataFormat::BC5;
      else if (header.ddspf.fourCC == DDSPF_BC5_SNORM.fourCC)
        format = DataFormat::BC5_SNORM;
      else if (header.ddspf.fourCC == DDSPF_A8R8G8B8.fourCC)
        format = DataFormat::RGBA_8;
      else if (header.ddspf.fourCC == DDSPF_A8B8G8R8.fourCC)
        format = DataFormat::BGRA_8;
      else
        ASSERT(false, "Missing implementation");
    }
  }

  const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(format);
  
  uint64 pitchSizeBytes = 0;
  if (formatInfo.myIsCompressed)
    pitchSizeBytes = static_cast<uint64>(glm::max(1u, MathUtil::DivideRoundUp(width, 4u)) * formatInfo.myCompressedBlockSizeBytes);
  else
    pitchSizeBytes = static_cast<uint64>(MathUtil::DivideRoundUp(width * static_cast<uint>(formatInfo.mySizeBytes), 8u));

  const uint heightBlocksOrPixels = formatInfo.myIsCompressed ? MathUtil::DivideRoundUp(height, 4u) : height;
  const uint64 dataSizeMip0 = heightBlocksOrPixels * pitchSizeBytes;

  eastl::vector<uint8> dataMip0;
  dataMip0.resize(dataSizeMip0);

  if (fread(dataMip0.data(), 1, dataSizeMip0, file) != dataSizeMip0)
  {
    LOG_ERROR("Error reading %d MiB from dds file %s", dataSizeMip0* SIZE_MB, aPathAbs);
    return false;
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