#pragma once
#include "Test.h"

#include "Rendering/ResourceHandle.h"
#include "EASTL/vector.h"
#include "EASTL/string.h"

namespace Fancy {
  class ShaderPipeline;
  class Texture;
  class TextureView;
}  // namespace Fancy

struct ImageData {
  ImageData( Fancy::TextureViewHandle aTexture ) {
    Create( aTexture );
  }
  ImageData() {}
  void Create( Fancy::TextureViewHandle aTexture );
  void Clear() {
    myTextureView = Fancy::TextureViewHandle{};
    myMipLevelReadViews.clear();
    myMipLevelWriteViews.clear();
  }
  bool myIsSRGB = false;

  Fancy::TextureViewHandle                  myTextureView;
  eastl::vector< Fancy::TextureViewHandle > myMipLevelReadViews;
  eastl::vector< Fancy::TextureViewHandle > myMipLevelWriteViews;

  eastl::string myName;
  bool          myIsWindowOpen;
  bool          myIsDirty;
  int           mySelectedMipLevel;
  int           mySelectedFilter;
};

class Test_Mipmapping : public Test {
public:
  Test_Mipmapping( Fancy::AssetManager * anAssetManager, Fancy::Window * aWindow, Fancy::RenderOutput * aRenderOutput, Fancy::InputState * anInputState );
  ~Test_Mipmapping() override;
  void OnUpdate( bool aDrawProperties ) override;

private:
  void OnShaderPipelineRecompiled( const Fancy::ShaderPipeline * aShader );

  eastl::vector< ImageData > myImageDatas;
  int                        mySelectedMipLevel = 0;
  bool                       myUpdateAlways = false;
};
