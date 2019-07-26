#pragma once
#include "Test.h"
#include "fancy_core/Ptr.h"
#include "fancy_core/DynamicArray.h"

namespace Fancy
{
  class Texture;
  class TextureView;
  class AssetManager;
  class GpuProgram;
}

struct ImageData
{
  ImageData(Fancy::SharedPtr<Fancy::Texture> aTexture) { Create(aTexture); }
  ImageData() {}
  void Create(Fancy::SharedPtr<Fancy::Texture> aTexture);
  void Clear() { myTexture.reset(); myTextureView.reset(); myMipLevelReadViews.clear(); myMipLevelWriteViews.clear(); }
  bool myIsSRGB = false;

  Fancy::SharedPtr<Fancy::Texture> myTexture;
  Fancy::SharedPtr<Fancy::TextureView> myTextureView;
  Fancy::DynamicArray<Fancy::SharedPtr<Fancy::TextureView>> myMipLevelReadViews;
  Fancy::DynamicArray<Fancy::SharedPtr<Fancy::TextureView>> myMipLevelWriteViews;

  Fancy::String myName;
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
  void OnShaderRecompiled(const Fancy::GpuProgram* aShader);

  Fancy::SharedPtr<Fancy::AssetManager> myAssetManager;
  Fancy::DynamicArray<ImageData> myImageDatas;
  int mySelectedMipLevel = 0;
  bool myUpdateAlways = false;
};

