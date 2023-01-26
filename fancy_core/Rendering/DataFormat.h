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
    DataFormatInfo(DataFormat aFormat, float aSizeBytes, float aSizeBytesPlane0, float aSizeBytesPlane1, uint aNumComponents, uint aNumPlanes, bool anIsDepthStencil = false, bool anSRGB = false, bool anIsCompressed = false, uint aIsUintInt = 0, uint aBlockSizeBytes = 0)
      : mySizeBytes(aSizeBytes)
      , myCopyableSizePerPlane{ aSizeBytesPlane0, aSizeBytesPlane1 }
      , myNumComponents(aNumComponents)
      , myNumPlanes(aNumPlanes)
      , myIsUintInt(aIsUintInt)
      , myFormat(aFormat)
      , myIsDepthStencil(anIsDepthStencil)
      , mySRGB(anSRGB)
      , myIsCompressed(anIsCompressed)
      , myCompressedBlockSizeBytes(aBlockSizeBytes)
    {}

    DataFormatInfo()
      : mySizeBytes(0.0f)
      , myCopyableSizePerPlane{ 0.0f, 0.0f }
      , myNumComponents(0u)
      , myNumPlanes(1u)
      , myIsUintInt(0u)
      , myFormat(NONE)
      , myIsDepthStencil(false)
      , mySRGB(false)
      , myIsCompressed(false)
      , myCompressedBlockSizeBytes(0)
    {}

    explicit DataFormatInfo(DataFormat aFormat);
    
    float mySizeBytes;
    float myCopyableSizePerPlane[2];
    uint myNumComponents;
    uint myNumPlanes;
    uint myIsUintInt; // 0: false, 1: Uint, 2: Iint
    DataFormat myFormat;
    bool myIsDepthStencil;
    bool mySRGB;
    bool myIsCompressed;
    uint myCompressedBlockSizeBytes;

    static const DataFormatInfo& GetFormatInfo(DataFormat aFormat);
    static DataFormat GetSRGBformat(DataFormat aFormat);
    static DataFormat GetNonSRGBformat(DataFormat aFormat);
  };
//---------------------------------------------------------------------------//
}

