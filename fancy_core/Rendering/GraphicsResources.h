#pragma once

#include "GpuBuffer.h"
#include "Texture.h"
#include "Common/Ptr.h"
#include "ResourceHandle.h"

namespace Fancy {
  //---------------------------------------------------------------------------//
  struct GpuBufferResourceProperties {
    GpuBufferResourceProperties()
        : myFormat( DataFormat::UNKNOWN ), myStructureSize( 0u ), myIsStructured( false ), myIsRaw( false ),
          myIsShaderResource( true ), myIsShaderWritable( false ) {}

    GpuBufferProperties myBufferProperties;

    DataFormat myFormat;
    uint       myStructureSize;
    bool       myIsStructured;
    bool       myIsRaw;

    bool myIsShaderResource;
    bool myIsShaderWritable;
  };
  //---------------------------------------------------------------------------//
  struct GpuBufferResource {
    void Update( const GpuBufferResourceProperties & someProps, const char * aName = nullptr );

    GpuBufferHandle     myBuffer;
    GpuBufferViewHandle myReadView;
    GpuBufferViewHandle myWriteView;
  };
  //---------------------------------------------------------------------------//

  //---------------------------------------------------------------------------//
  struct TextureResourceProperties {
    TextureResourceProperties() : myIsTexture( true ), myIsShaderWritable( false ), myIsRenderTarget( false ) {}

    TextureProperties myTextureProperties;
    bool              myIsTexture;
    bool              myIsShaderWritable;
    bool              myIsRenderTarget;
  };
  //---------------------------------------------------------------------------//
  struct TextureResource {
    void Update( const TextureResourceProperties & someProps, const char * aName = nullptr );

    TextureHandle     myTexture;
    TextureViewHandle myReadView;
    TextureViewHandle myWriteView;
    TextureViewHandle myRenderTargetView;
  };
  //---------------------------------------------------------------------------//
}  // namespace Fancy