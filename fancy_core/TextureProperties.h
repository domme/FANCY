#pragma once

#include "FancyCoreDefines.h"
#include "DataFormat.h"
#include "RenderEnums.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct TextureProperties
  {
    TextureProperties()
      : myDimension(GpuResourceDimension::UNKONWN)
      , path("")
      , myWidth(0u)
      , myHeight(0u)
      , myDepthOrArraySize(1u)
      , eFormat(DataFormat::NONE)
      , myAccessType(CpuMemoryAccessType::NO_CPU_ACCESS)
      , myNumMipLevels(UINT_MAX)
      , bIsDepthStencil(false)
      , myIsShaderWritable(false)
      , myIsRenderTarget(false)
      , myPreferTypedFormat(false)
    {}

    bool IsArray() const { return myDimension == GpuResourceDimension::TEXTURE_1D_ARRAY || myDimension == GpuResourceDimension::TEXTURE_2D_ARRAY || myDimension == GpuResourceDimension::TEXTURE_CUBE_ARRAY; }
    uint GetArraySize() const { return IsArray() ? myDepthOrArraySize : 1u; }
    uint GetDepthSize() const { return IsArray() ? 1u : myDepthOrArraySize; }

    GpuResourceDimension myDimension;
    String path;
    uint myWidth;
    uint myHeight;
    uint myDepthOrArraySize;
    DataFormat eFormat;
    CpuMemoryAccessType myAccessType;
    uint myNumMipLevels;
    bool bIsDepthStencil;
    bool myIsShaderWritable;
    bool myIsRenderTarget;
    bool myPreferTypedFormat;
  };
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
      , myPlaneIndex(0u)  // TODO(VK): Exchange simple plane index for an enum-value that specifies the semantic of the plane so that it can work well with VkImageSubresourceRange
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
