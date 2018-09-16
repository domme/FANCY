#pragma once

#include "FancyCorePrerequisites.h"

namespace Fancy {
//---------------------------------------------------------------------------//  
  struct TextureViewProperties
  {
    TextureViewProperties()
      : myDimension(GpuResourceDimension::UNKONWN)
      , myFormat(DataFormat::UNKNOWN)
      , myIsShaderWritable(false)
      , myIsRenderTarget(false)
      , myIsDepthReadOnly(false)
      , myIsStencilReadOnly(false)
      , myNumMipLevels(UINT_MAX)
      , myPlaneIndex(0u)
      , myArraySize(1u)
      , myFirstArrayIndex(0u)
      , myMinLodClamp(0.0f)
      , myMipIndex(0u)
      , myFirstZindex(0u)
      , myZSize(1u)
    { }

    GpuResourceDimension myDimension;
    DataFormat myFormat;
    bool myIsShaderWritable;
    bool myIsRenderTarget;
    bool myIsDepthReadOnly;
    bool myIsStencilReadOnly;
    uint myNumMipLevels;
    uint myPlaneIndex;
    uint myArraySize;        // Interpreted as NumCubes in case of cube arrays
    uint myFirstArrayIndex;  // Interpreted as First 2D Array face in case of cube arrays
    float myMinLodClamp;
    uint myMipIndex; // Only rendertargets
    uint myFirstZindex;  // Only rendertargets
    uint myZSize; // Only rendertargets
  };
//---------------------------------------------------------------------------//
}
