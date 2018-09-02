#include "FancyCorePrerequisites.h"
#include "DataFormat.h"

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
#define DECLARE_DATA_FORMAT(name, byteSize, numComponents, numPlanes, depthStencil, srgb, compressed) { DataFormat::name, byteSize, numComponents, numPlanes, depthStencil, srgb, compressed },
      #include "DataFormatList.h"
#undef DECLARE_DATA_FORMAT
    };

    STATIC_ASSERT(ARRAY_LENGTH(locOurDataFormats) == DataFormat::NUM, "DataFormatInfos don't match with the number of DataFormats");
    return locOurDataFormats[static_cast<uint>(aFormat)];
  }
//---------------------------------------------------------------------------//
  DataFormat DataFormatInfo::GetSRGBformat(DataFormat aFormat)
  {
    switch (aFormat)
    {
      case DataFormat::RGBA_8: return DataFormat::SRGB_8_A_8;
    }

    ASSERT(GetFormatInfo(aFormat).mySRGB, "Missing implementation or no SRGB format available");
    return aFormat;
  }
//---------------------------------------------------------------------------//
  DataFormat DataFormatInfo::GetNonSRGBformat(DataFormat aFormat)
  {
    switch (aFormat)
    {
      case DataFormat::SRGB_8_A_8: return DataFormat::RGBA_8;
    }

    ASSERT(!GetFormatInfo(aFormat).mySRGB, "Missing implementation");
    return aFormat;
  }
//---------------------------------------------------------------------------//
}