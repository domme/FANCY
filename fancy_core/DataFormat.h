#pragma once

namespace Fancy {
//---------------------------------------------------------------------------//
  enum DataFormat
  {
#define DECLARE_DATA_FORMAT(name, ...) name,
  #include "DataFormatList.h"
#undef DECLARE_DATA_FORMAT
    NUM,
    UNKNOWN = NONE
  };
//---------------------------------------------------------------------------//
  class DataFormatInfo
  {
  public:
    DataFormatInfo(DataFormat aFormat, uint aSizeBytes, uint aNumComponents, bool anIsDepthStencil = false, bool anSRGB = false, bool anIsCompressed = false)
      : mySizeBytes(aSizeBytes)
      , myNumComponents(aNumComponents)
      , myFormat(aFormat)
      , myIsDepthStencil(anIsDepthStencil)
      , mySRGB(anSRGB)
      , myIsCompressed(anIsCompressed)
    {}

    DataFormatInfo()
      : mySizeBytes(0u)
      , myNumComponents(0u)
      , myFormat(NONE)
      , mySRGB(false)
      , myIsCompressed(false)
      , myIsDepthStencil(false)
    {}

    explicit DataFormatInfo(DataFormat aFormat);
    
    uint mySizeBytes;
    uint myNumComponents;
    DataFormat myFormat;
    bool myIsDepthStencil : 1;
    bool mySRGB : 1;
    bool myIsCompressed : 1;
    int __padding : 1;

    static const DataFormatInfo& GetFormatInfo(DataFormat aFormat);
    static DataFormat GetSRGBformat(DataFormat aFormat);
    static DataFormat GetNonSRGBformat(DataFormat aFormat);
  };
//---------------------------------------------------------------------------//
}
