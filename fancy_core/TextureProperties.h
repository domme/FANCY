#pragma once

#include "FancyCoreDefines.h"
#include "DataFormat.h"
#include "RenderEnums.h"
#include "TextureData.h"

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
      , myFormat(DataFormat::NONE)
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
    DataFormat myFormat;
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
      , myMinLodClamp(0.0f)
      , myFirstZindex(0u)
      , myZSize(1u)
    { }

    SubresourceRange mySubresourceRange;
    GpuResourceDimension myDimension;
    DataFormat myFormat;
    bool myIsShaderWritable;
    bool myIsRenderTarget;
    bool myIsDepthReadOnly;
    bool myIsStencilReadOnly;
    float myMinLodClamp;
    uint myFirstZindex;  // Only rendertargets
    uint myZSize; // Only rendertargets
  };
//---------------------------------------------------------------------------//
}
