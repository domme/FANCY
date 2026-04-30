#include "fancy_core_precompile.h"
#include "GraphicsResources.h"

#include "RenderCore.h"

namespace Fancy {
  //---------------------------------------------------------------------------//
  void GpuBufferResource::Update( const GpuBufferResourceProperties & someProps, const char * aName ) {
    const GpuBufferProperties & newProps = someProps.myBufferProperties;
    ASSERT( !someProps.myIsShaderWritable || newProps.myIsShaderWritable );

    bool needsCreate = !myBuffer.IsValid() || ( someProps.myIsShaderResource == !myReadView.IsValid() ) ||
                       ( someProps.myIsShaderWritable == !myWriteView.IsValid() );

    if ( !needsCreate ) {
      const GpuBufferProperties & currProps = RenderCore::GetBuffer( myBuffer )->GetProperties();
      needsCreate = newProps.myElementSizeBytes != currProps.myElementSizeBytes ||
                    newProps.myNumElements != currProps.myNumElements ||
                    newProps.myCpuAccess != currProps.myCpuAccess || newProps.myBindFlags != currProps.myBindFlags;

      if ( myReadView.IsValid() || myWriteView.IsValid() ) {
        const GpuBufferViewProperties & currViewProps = myReadView.IsValid()
                                                            ? RenderCore::GetBufferView( myReadView )->GetProperties()
                                                            : RenderCore::GetBufferView( myWriteView )->GetProperties();

        needsCreate |= currViewProps.myIsRaw != someProps.myIsRaw || currViewProps.myFormat != someProps.myFormat ||
                       currViewProps.myIsStructured != someProps.myIsStructured;
      }
    }

    if ( needsCreate ) {
      myBuffer = RenderCore::CreateBuffer( newProps, aName );
      ASSERT( myBuffer.IsValid() );

      GpuBufferViewProperties viewProps;
      viewProps.myFormat = someProps.myFormat;
      viewProps.myIsRaw = someProps.myIsRaw;
      viewProps.myIsStructured = someProps.myIsStructured;

      if ( someProps.myIsShaderResource ) {
        myReadView = RenderCore::CreateBufferView( RenderCore::GetBuffer( myBuffer ), viewProps );
        ASSERT( myReadView.IsValid() );
      }

      if ( someProps.myIsShaderWritable ) {
        viewProps.myIsShaderWritable = true;
        myWriteView = RenderCore::CreateBufferView( RenderCore::GetBuffer( myBuffer ), viewProps );
        ASSERT( myWriteView.IsValid() );
      }
    }

    RenderCore::GetBuffer( myBuffer )->SetName( aName );
  }
  //---------------------------------------------------------------------------//

  //---------------------------------------------------------------------------//
  void TextureResource::Update( const TextureResourceProperties & someProps, const char * aName ) {
    const TextureProperties & texProps = someProps.myTextureProperties;

    bool needsCreate = !myTexture.IsValid() || ( someProps.myIsTexture == !myReadView.IsValid() ) ||
                       ( someProps.myIsRenderTarget == !myRenderTargetView.IsValid() ) ||
                       ( someProps.myIsShaderWritable == !myWriteView.IsValid() );

    if ( !needsCreate ) {
      const TextureProperties & currTexProps = RenderCore::GetTexture( myTexture )->GetProperties();
      needsCreate = texProps.myWidth != currTexProps.myWidth || texProps.myHeight != currTexProps.myHeight ||
                    texProps.GetDepthSize() != currTexProps.GetDepthSize() ||
                    texProps.GetArraySize() != currTexProps.GetArraySize() ||
                    texProps.myFormat != currTexProps.myFormat || texProps.myDimension != currTexProps.myDimension ||
                    texProps.myPreferTypedFormat != currTexProps.myPreferTypedFormat;
    }

    if ( needsCreate ) {
      TextureProperties createProps = texProps;
      createProps.myPath.clear();
      createProps.myIsShaderWritable = someProps.myIsShaderWritable;
      createProps.myIsRenderTarget = someProps.myIsRenderTarget;

      myTexture = RenderCore::CreateTexture( createProps, aName );
      ASSERT( myTexture.IsValid() );

      if ( someProps.myIsTexture ) {
        const TextureViewProperties viewProps;
        myReadView = RenderCore::CreateTextureView( RenderCore::GetTexture( myTexture ), viewProps );
        ASSERT( myReadView.IsValid() );
      }

      if ( someProps.myIsRenderTarget ) {
        TextureViewProperties viewProps;
        viewProps.myIsRenderTarget = true;
        viewProps.mySubresourceRange.myFirstMipLevel = 0u;
        viewProps.mySubresourceRange.myNumMipLevels = 1u;
        myRenderTargetView = RenderCore::CreateTextureView( RenderCore::GetTexture( myTexture ), viewProps );
        ASSERT( myRenderTargetView.IsValid() );
      }

      if ( someProps.myIsShaderWritable ) {
        TextureViewProperties viewProps;
        viewProps.myIsShaderWritable = true;
        viewProps.mySubresourceRange.myFirstMipLevel = 0u;
        viewProps.mySubresourceRange.myNumMipLevels = 1u;
        viewProps.myFormat = DataFormatInfo::GetNonSRGBformat( someProps.myTextureProperties.myFormat );
        myWriteView = RenderCore::CreateTextureView( RenderCore::GetTexture( myTexture ), viewProps );
        ASSERT( myWriteView.IsValid() );
      }
    }

    RenderCore::GetTexture( myTexture )->SetName( aName );
  }
  //---------------------------------------------------------------------------//
}  // namespace Fancy