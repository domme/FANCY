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
    DataFormatInfo(DataFormat aFormat, uint aSizeBytes, uint aSizeBytesPlane0, uint aSizeBytesPlane1, uint aNumComponents, uint aNumPlanes, bool anIsDepthStencil = false, bool anSRGB = false, bool anIsCompressed = false)
      : mySizeBytes(aSizeBytes)
      , mySizeBytesPerPlane{ aSizeBytesPlane0, aSizeBytesPlane1 }
      , myNumComponents(aNumComponents)
      , myNumPlanes(aNumPlanes)
      , myFormat(aFormat)
      , myIsDepthStencil(anIsDepthStencil)
      , mySRGB(anSRGB)
      , myIsCompressed(anIsCompressed)
    {}

    DataFormatInfo()
      : mySizeBytes(0u)
      , myNumComponents(0u)
      , myNumPlanes(1u)
      , myFormat(NONE)
      , mySRGB(false)
      , myIsCompressed(false)
      , myIsDepthStencil(false)
    {}

    explicit DataFormatInfo(DataFormat aFormat);
    
    uint mySizeBytes;
    uint mySizeBytesPerPlane[2];
    uint myNumComponents;
    uint myNumPlanes;
    DataFormat myFormat;
    bool myIsDepthStencil;
    bool mySRGB;
    bool myIsCompressed;

    static const DataFormatInfo& GetFormatInfo(DataFormat aFormat);
    static DataFormat GetSRGBformat(DataFormat aFormat);
    static DataFormat GetNonSRGBformat(DataFormat aFormat);
  };
//---------------------------------------------------------------------------//
}
