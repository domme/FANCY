#include "Test_Mipmapping.h"
#include "Rendering/TextureProperties.h"
#include "Rendering/Texture.h"
#include "imgui.h"
#include "Common/Ptr.h"
#include "Debug/Log.h"
#include "IO/Assets.h"
#include "Rendering/RenderCore.h"
#include "Rendering/TextureProperties.h"

using namespace Fancy;

static const char * locResampleFilterNames[] = { "Linear", "Lanczos" };

void ImageData::Create( Fancy::TextureViewHandle aTexture ) {
  TextureView * textureView = RenderCore::GetTextureView( aTexture );
  Texture * texture = textureView->GetTexture();
  const TextureProperties & destTexProps = texture->GetProperties();
  if ( destTexProps.myNumMipLevels == 1 )
    return;

  myTextureView = aTexture;
  TextureViewProperties readProps;
  readProps.myFormat = textureView->GetProperties().myFormat;
  readProps.myDimension = GpuResourceDimension::TEXTURE_2D;

  const DataFormatInfo & destTexFormatInfo = DataFormatInfo::GetFormatInfo( destTexProps.myFormat );
  myIsSRGB = destTexFormatInfo.mySRGB;

  readProps.mySubresourceRange.myNumMipLevels = 1;

  TextureViewProperties writeProps = readProps;
  writeProps.myFormat = DataFormatInfo::GetNonSRGBformat( readProps.myFormat );
  writeProps.myIsShaderWritable = true;

  const uint numMips = destTexProps.myNumMipLevels;
  myMipLevelReadViews.resize( numMips );
  myMipLevelWriteViews.resize( numMips );

  for ( uint mip = 0u; mip < numMips; ++mip ) {
    readProps.mySubresourceRange.myFirstMipLevel = mip;
    writeProps.mySubresourceRange.myFirstMipLevel = mip;
    myMipLevelReadViews[ mip ] = RenderCore::CreateTextureView( texture, readProps );
    myMipLevelWriteViews[ mip ] = RenderCore::CreateTextureView( texture, writeProps );
  }

  eastl::string texturePath = destTexProps.myPath;
  myName = texturePath.substr( texturePath.find_last_of( '/' ) + 1 );
  myIsWindowOpen = false;
  myIsDirty = false;
  mySelectedMipLevel = 0;
  mySelectedFilter = Assets::FILTER_LINEAR;
}

Test_Mipmapping::Test_Mipmapping( Fancy::AssetManager * anAssetManager, Fancy::Window * aWindow, Fancy::RenderOutput * aRenderOutput,
                                  Fancy::InputState * anInputState )
    : Test( anAssetManager, aWindow, aRenderOutput, anInputState, "Mipmapping" ) {
  const uint loadFlags = Assets::SHADER_WRITABLE;
  myImageDatas.push_back( Assets::LoadTexture( "Textures/Sibenik/kamen.png", loadFlags ) );
  myImageDatas.push_back( Assets::LoadTexture( "Textures/Checkerboard.png", loadFlags ) );
  myImageDatas.push_back( Assets::LoadTexture( "Textures/Sibenik/mramor6x6.png", loadFlags ) );

  RenderCore::ourOnShaderPipelineRecompiled.Connect( this, &Test_Mipmapping::OnShaderPipelineRecompiled );
}

Test_Mipmapping::~Test_Mipmapping() {
  RenderCore::ourOnShaderPipelineRecompiled.DetachObserver( this );
}

void Test_Mipmapping::OnUpdate( bool aDrawProperties ) {
  if ( !aDrawProperties )
    return;

  const uint numTextures = ( uint ) myImageDatas.size();
  ImGui::Checkbox( "Update every frame", &myUpdateAlways );

  for ( uint i = 0u; i < numTextures; ++i ) {
    ImageData &   data = myImageDatas[ i ];
    TextureView * textureView = RenderCore::GetTextureView( data.myTextureView );
    Texture *     texture = textureView->GetTexture();
    const TextureProperties & texProps = texture->GetProperties();

    ImGui::Checkbox( data.myName.c_str(), &data.myIsWindowOpen );
    if ( data.myIsWindowOpen ) {
      ImGui::SetNextWindowSize( ImVec2( static_cast< float >( texProps.myWidth * 2 ), static_cast< float >( texProps.myHeight * 2 ) ) );
      ImGui::Begin( data.myName.c_str() );
      ImGui::SliderInt( "Mip Level", &data.mySelectedMipLevel, 0, texProps.myNumMipLevels - 1 );
      data.myIsDirty |= ImGui::ListBox( "Downsample Filter", &data.mySelectedFilter, locResampleFilterNames, ARRAY_LENGTH( locResampleFilterNames ) );

      if ( data.myIsDirty | myUpdateAlways ) {
        Assets::ComputeMipmaps( data.myTextureView.mySourceTexture, ( Assets::ResampleFilter ) data.mySelectedFilter );
        data.myIsDirty = false;
      }

      TextureView * mipTextureView = RenderCore::GetTextureView( data.myMipLevelReadViews[ data.mySelectedMipLevel ] );
      ImGui::Image( ( ImTextureID ) mipTextureView,
                    ImVec2( static_cast< float >( texProps.myWidth ), static_cast< float >( texProps.myHeight ) ) );
      ImGui::End();
    }
  }
}

void Test_Mipmapping::OnShaderPipelineRecompiled( const Fancy::ShaderPipeline * aShader ) {
  if ( aShader == Assets::GetMipDownsampleShader() ) {
    for ( ImageData & data : myImageDatas )
      data.myIsDirty = true;
  }
}
