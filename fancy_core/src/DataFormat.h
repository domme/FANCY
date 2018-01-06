#pragma once

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  enum DataFormat {
    NONE = 0,
    SRGB_8_A_8,
    RGBA_8,
    SRGB_8,
    RGB_8,
    RGB_11_11_10F,
    RGBA_16F,
    RGB_16F,
    RG_16F,
    R_16F,
    RGBA_32F,
    RGB_32F,
    RG_32F,
    R_32F,
    RGBA_32UI,
    RGB_32UI,
    RG_32UI,
    R_32UI,
    RGBA_16UI,
    RGB_16UI,
    RG_16UI,
    R_16UI,
    RGBA_8UI,
    RGB_8UI,
    RG_8UI,
    R_8UI,
    RGBA_32I,
    RGB_32I,
    RG_32I,
    R_32I,
    RGBA_16I,
    RGB_16I,
    RG_16I,
    R_16I,
    RGBA_8I,
    RGB_8I,
    RG_8I,
    R_8I,
    DS_24_8,

    // Compressed formats go here...
    // TODO: Find a way to declare compressed formats

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
  };
//---------------------------------------------------------------------------//
} }  // Fancy::Rendering
