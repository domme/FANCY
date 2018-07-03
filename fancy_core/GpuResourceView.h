#pragma once

#include "RendererPrerequisites.h"
#include "GpuResource.h"

namespace Fancy {
  //---------------------------------------------------------------------------//
  struct GpuResourceViewProperties
  {
    GpuResourceViewProperties()
      : myDimension(GpuResourceDimension::UNKONWN)
      , myFormat(DataFormat::NONE)
      , myIsShaderWritable(false)
      , myIsRenderTarget(false)
      , myNumMipLevels(1u)
      , myPlaneIndex(0u)
      , myArraySize(0u)
      , myFirstArrayIndex(0u)
      , myMinLodClamp(0.0f)
      , myMipIndex(0u)
      , myFirstZindex(0u)
      , myZSize(0u)
    { }

    GpuResourceDimension myDimension;
    DataFormat myFormat;
    bool myIsShaderWritable;
    bool myIsRenderTarget;
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
  class GpuResourceView
  {
  public:
    const GpuResourceViewProperties& GetProperties() const { return myProperties; }
    GpuResource* GetResource() const { return myResource.get(); }

  protected:
    GpuResourceViewProperties myProperties;
    SharedPtr<GpuResource> myResource;
  };
  //---------------------------------------------------------------------------//
}
