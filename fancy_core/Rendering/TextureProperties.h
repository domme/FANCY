#pragma once

#include "Common/FancyCoreDefines.h"

#include "DataFormat.h"
#include "RenderEnums.h"
#include "TextureData.h"

#include "EASTL/string.h"

namespace Fancy {
  //---------------------------------------------------------------------------//
  struct TextureProperties {
    TextureProperties()
        : myDimension( GpuResourceDimension::TEXTURE_2D ), myPath( "" ), myWidth( 0u ), myHeight( 0u ),
          myDepthOrArraySize( 1u ), myFormat( DataFormat::NONE ), myAccessType( CpuMemoryAccessType::NO_CPU_ACCESS ),
          myNumMipLevels( 1u ), bIsDepthStencil( false ), myIsShaderWritable( false ), myIsRenderTarget( false ),
          myPreferTypedFormat( false ), myAllowCrossQueueAccess( false ) {}

    bool IsArray() const {
      return myDimension == GpuResourceDimension::TEXTURE_1D_ARRAY ||
             myDimension == GpuResourceDimension::TEXTURE_2D_ARRAY ||
             myDimension == GpuResourceDimension::TEXTURE_CUBE_ARRAY;
    }
    uint GetArraySize() const {
      return IsArray() ? myDepthOrArraySize : 1u;
    }
    uint GetDepthSize() const {
      return IsArray() ? 1u : myDepthOrArraySize;
    }
    void GetSize( uint aMipLevel, uint & aWidthOut, uint & aHeightOut, uint & aDepthOut ) const;

    GpuResourceDimension myDimension;
    eastl::string        myPath;
    uint                 myWidth;
    uint                 myHeight;
    uint                 myDepthOrArraySize;
    DataFormat           myFormat;
    CpuMemoryAccessType  myAccessType;
    uint                 myNumMipLevels;
    bool                 bIsDepthStencil;
    bool                 myIsShaderWritable;
    bool                 myIsRenderTarget;
    bool                 myPreferTypedFormat;
    // When true: creates the texture with D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS.
    // The texture is permanently in COMMON layout, meaning any queue type (graphics, compute,
    // copy) can access it as SRV or UAV without TextureBarrier layout transitions.
    // After a UAV write, only a fence sync + GlobalBarrier(ShaderWrite) is needed before
    // the reading queue accesses it — no TextureBarrier required.
    //
    // Trade-off: disables GPU lossless compression (DCC on AMD RDNA2+, delta color compression
    // on NVIDIA Ampere+) for this resource. This is typically zero cost for float UAV compute
    // textures (no compression applies anyway) but has real impact on RGBA8 render targets.
    //
    // Constraints: Cannot be combined with bIsDepthStencil (D3D12 restriction) or MSAA (SampleDesc.Count > 1).
    bool myAllowCrossQueueAccess;
  };
  //---------------------------------------------------------------------------//
  struct TextureViewProperties {
    TextureViewProperties()
        : myDimension( GpuResourceDimension::UNKONWN ), myFormat( DataFormat::UNKNOWN ), myIsShaderWritable( false ),
          myIsRenderTarget( false ), myIsDepthReadOnly( false ), myIsStencilReadOnly( false ), myMinLodClamp( 0.0f ),
          myFirstZindex( 0u ), myZSize( 1u ) {}

    SubresourceRange     mySubresourceRange;
    GpuResourceDimension myDimension;
    DataFormat           myFormat;
    bool                 myIsShaderWritable;
    bool                 myIsRenderTarget;
    bool                 myIsDepthReadOnly;
    bool                 myIsStencilReadOnly;
    float                myMinLodClamp;
    uint                 myFirstZindex;  // Only rendertargets
    uint                 myZSize;        // Only rendertargets
  };
  //---------------------------------------------------------------------------//
}  // namespace Fancy
