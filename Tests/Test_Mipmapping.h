#pragma once
#include "Test.h"

#include "Common/Ptr.h"
#include "EASTL/vector.h"
#include "EASTL/string.h"

namespace Fancy
{
  class ShaderPipeline;
  class Texture;
  class TextureView;
}

struct ImageData
{
  ImageData(Fancy::SharedPtr<Fancy::TextureView> aTexture) { Create(aTexture); }
  ImageData() {}
  void Create(Fancy::SharedPtr<Fancy::TextureView> aTexture);
  void Clear() { myTexture.reset(); myTextureView.reset(); myMipLevelReadViews.clear(); myMipLevelWriteViews.clear(); }
  bool myIsSRGB = false;

  Fancy::SharedPtr<Fancy::Texture> myTexture;
  Fancy::SharedPtr<Fancy::TextureView> myTextureView;
  eastl::vector<Fancy::SharedPtr<Fancy::TextureView>> myMipLevelReadViews;
  eastl::vector<Fancy::SharedPtr<Fancy::TextureView>> myMipLevelWriteViews;

  eastl::string myName;
  bool myIsWindowOpen;
  bool myIsDirty;
  int mySelectedMipLevel;
  int mySelectedFilter;
};

class Test_Mipmapping : public Test
{
public:
  Test_Mipmapping(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState);
  ~Test_Mipmapping() override;
  void OnUpdate(bool aDrawProperties) override;

private:
  void OnShaderPipelineRecompiled(const Fancy::ShaderPipeline* aShader);
  
  eastl::vector<ImageData> myImageDatas;
  int mySelectedMipLevel = 0;
  bool myUpdateAlways = false;
};

