#include "fancy_core_precompile.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  DataFormatInfo::DataFormatInfo(DataFormat aFormat)
  {
    *this = GetFormatInfo(aFormat);
  }
//---------------------------------------------------------------------------//
  const DataFormatInfo& DataFormatInfo::GetFormatInfo(DataFormat aFormat)
  {
    static DataFormatInfo locOurDataFormats[static_cast<uint>(DataFormat::NUM)] =
    {
#define DECLARE_DATA_FORMAT(name, byteSize, byteSizePlane0, byteSizePlane1, numComponents, numPlanes, depthStencil, srgb, compressed, isUintInt, blockSize) { DataFormat::name, byteSize, byteSizePlane0, byteSizePlane1, numComponents, numPlanes, depthStencil, srgb, compressed, isUintInt, blockSize },
      #include "DataFormatList.h"
#undef DECLARE_DATA_FORMAT
    };

    STATIC_ASSERT(ARRAY_LENGTH(locOurDataFormats) == DataFormat::NUM, "DataFormatInfos don't match with the number of DataFormats");
    return locOurDataFormats[static_cast<uint>(aFormat)];
  }
//---------------------------------------------------------------------------//
  DataFormat DataFormatInfo::GetSRGBformat(DataFormat aFormat)
  {
    switch( aFormat )
    {
    case RGBA_8: return SRGB_8_A_8;
    case BC1: return BC1_SRGB;
    case BC3: return BC3_SRGB;
    case BC7: return BC7_SRGB;
      default:
        ASSERT(GetFormatInfo(aFormat).mySRGB, "Missing implementation or no SRGB format available");
        return aFormat;
    }
  }
//---------------------------------------------------------------------------//
  DataFormat DataFormatInfo::GetNonSRGBformat(DataFormat aFormat)
  {
    switch (aFormat)
    {
    case SRGB_8_A_8: return RGBA_8;
    case BC1_SRGB: return BC1;
    case BC3_SRGB: return BC3;
    case BC7_SRGB: return BC7;
    default:
      ASSERT(GetFormatInfo(aFormat).mySRGB, "Missing implementation or no SRGB format available");
      return aFormat;
    }
  }
//---------------------------------------------------------------------------//
}
