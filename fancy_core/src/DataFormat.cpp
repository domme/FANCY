#include "FancyCorePrerequisites.h"
#include "DataFormat.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  DataFormatInfo::DataFormatInfo(DataFormat aFormat)
  {
    *this = GetFormatInfo(aFormat);
  }
//---------------------------------------------------------------------------//
  const DataFormatInfo& DataFormatInfo::GetFormatInfo(DataFormat aFormat)
  {
    static DataFormatInfo locOurDataFormats[static_cast<uint32>(DataFormat::NUM)] =
    {
      // Format,                 ByteSize,  NumComponents,  DepthStencil,  SRGB,    Compressed
      { DataFormat::NONE,          0,           0,             false,       false,     false},
      { DataFormat::SRGB_8_A_8,    4,           4,             false,       true,      false},
      { DataFormat::RGBA_8,        4,           4,             false,       false,     false},
      { DataFormat::SRGB_8,        3,           3,             false,       true,      false},
      { DataFormat::RGB_8,         3,           3,             false,       false,     false},
      { DataFormat::RGB_11_11_10F, 4,           3,             false,       false,     false},
      { DataFormat::RGBA_16F,      8,           4,             false,       false,     false},
      { DataFormat::RGB_16F,       6,           3,             false,       false,     false},
      { DataFormat::RG_16F,        4,           2,             false,       false,     false},
      { DataFormat::R_16F,         2,           1,             false,       false,     false},
      { DataFormat::RGBA_32F,     16,           4,             false,       false,     false},
      { DataFormat::RGB_32F,      12,           3,             false,       false,     false},
      { DataFormat::RG_32F,        8,           2,             false,       false,     false},
      { DataFormat::R_32F,         4,           1,             false,       false,     false},
      { DataFormat::RGBA_32UI,    16,           4,             false,       false,     false},
      { DataFormat::RGB_32UI,     12,           3,             false,       false,     false},
      { DataFormat::RG_32UI,       8,           2,             false,       false,     false},
      { DataFormat::R_32UI,        4,           1,             false,       false,     false},
      { DataFormat::RGBA_16UI,     8,           4,             false,       false,     false},
      { DataFormat::RGB_16UI,      6,           3,             false,       false,     false},
      { DataFormat::RG_16UI,       4,           2,             false,       false,     false},
      { DataFormat::R_16UI,        2,           1,             false,       false,     false},
      { DataFormat::RGBA_8UI,      4,           4,             false,       false,     false},
      { DataFormat::RGB_8UI,       3,           3,             false,       false,     false},
      { DataFormat::RG_8UI,        2,           2,             false,       false,     false},
      { DataFormat::R_8UI,         1,           1,             false,       false,     false},
      { DataFormat::RGBA_32I,     16,           4,             false,       false,     false},
      { DataFormat::RGB_32I,      12,           3,             false,       false,     false},
      { DataFormat::RG_32I,        8,           2,             false,       false,     false},
      { DataFormat::R_32I,         4,           1,             false,       false,     false},
      { DataFormat::RGBA_16I,      8,           4,             false,       false,     false},
      { DataFormat::RGB_16I,       6,           3,             false,       false,     false},
      { DataFormat::RG_16I,        4,           2,             false,       false,     false},
      { DataFormat::R_16I,         2,           1,             false,       false,     false},
      { DataFormat::RGBA_8I,       4,           4,             false,       false,     false},
      { DataFormat::RGB_8I,        3,           3,             false,       false,     false},
      { DataFormat::RG_8I,         2,           2,             false,       false,     false},
      { DataFormat::R_8I,          1,           1,             false,       false,     false},
      { DataFormat::DS_24_8,       4,           2,             true,        false,     false},
    };

    return locOurDataFormats[static_cast<uint32>(aFormat)];
  }
//---------------------------------------------------------------------------//
} }
