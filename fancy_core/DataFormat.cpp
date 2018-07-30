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
#define DECLARE_DATA_FORMAT(name, byteSize, numComponents, depthStencil, compressed) { DataFormat::name, byteSize, numComponents, depthStencil, compressed },
      #include "DataFormatList.h"
#undef DECLARE_DATA_FORMAT
    };

    STATIC_ASSERT(ARRAY_LENGTH(locOurDataFormats) == DataFormat::NUM, "DataFormatInfos don't match with the number of DataFormats");
    return locOurDataFormats[static_cast<uint>(aFormat)];
  }
//---------------------------------------------------------------------------//
}