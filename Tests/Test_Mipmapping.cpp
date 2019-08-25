#include "Test_Mipmapping.h"
#include "fancy_core/TextureProperties.h"
#include "fancy_core/Log.h"
#include "fancy_core/Texture.h"
#include "fancy_core/RenderCore.h"
#include "fancy_assets/AssetManager.h"
#include "fancy_imgui/imgui.h"

using namespace Fancy;

static const char* locResampleFilterNames[] = { "Linear", "Lanczos" };

void ImageData::Create(SharedPtr<Texture> aTexture)
{
  const TextureProperties& destTexProps = aTexture->GetProperties();
  if (destTexProps.myNumMipLevels == 1)
    return;

  myTexture = aTexture;
  TextureViewProperties readProps;
  readProps.myFormat = aTexture->GetProperties().eFormat;
  readProps.myDimension = GpuResourceDimension::TEXTURE_2D;
  myTextureView = RenderCore::CreateTextureView(aTexture, readProps);
  ASSERT(myTextureView != nullptr);

  const DataFormatInfo& destTexFormatInfo = DataFormatInfo::GetFormatInfo(destTexProps.eFormat);
  myIsSRGB = destTexFormatInfo.mySRGB;

  readProps.mySubresourceRange.myNumMipLevels = 1;

  TextureViewProperties writeProps = readProps;
  writeProps.myFormat = DataFormatInfo::GetNonSRGBformat(readProps.myFormat);
  writeProps.myIsShaderWritable = true;

  const uint numMips = destTexProps.myNumMipLevels;
  myMipLevelReadViews.resize(numMips);
  myMipLevelWriteViews.resize(numMips);

  for (uint mip = 0u; mip < numMips; ++mip)
  {
    readProps.mySubresourceRange.myFirstMipLevel = mip;
    writeProps.mySubresourceRange.myFirstMipLevel = mip;
    myMipLevelReadViews[mip] = RenderCore::CreateTextureView(aTexture, readProps);
    myMipLevelWriteViews[mip] = RenderCore::CreateTextureView(aTexture, writeProps);
    ASSERT(myMipLevelReadViews[mip] != nullptr && myMipLevelWriteViews[mip] != nullptr);
  }

  String texturePath = destTexProps.path;
  myName = texturePath.substr(texturePath.find_last_of('/') + 1);
  myIsWindowOpen = false;
  myIsDirty = false;
  mySelectedMipLevel = 0;
  mySelectedFilter = AssetManager::FILTER_LINEAR;
}


Test_Mipmapping::Test_Mipmapping(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow,
  Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState)
  : Test(aRuntime, aWindow, aRenderOutput, anInputState, "Mipmapping")
{
  myAssetManager.reset(new AssetManager());

  const uint loadFlags = AssetManager::SHADER_WRITABLE;
  myImageDatas.push_back(myAssetManager->CreateTexture("Textures/Sibenik/kamen.png", loadFlags));
  myImageDatas.push_back(myAssetManager->CreateTexture("Textures/Checkerboard.png", loadFlags));
  myImageDatas.push_back(myAssetManager->CreateTexture("Textures/Sibenik/mramor6x6.png", loadFlags));

  RenderCore::ourOnShaderRecompiled.Connect(this, &Test_Mipmapping::OnShaderRecompiled);

}

Test_Mipmapping::~Test_Mipmapping()
{
}

void Test_Mipmapping::OnUpdate(bool aDrawProperties)
{
  if (!aDrawProperties)
    return;

  const uint numTextures = (uint)myImageDatas.size();
  ImGui::Checkbox("Update every frame", &myUpdateAlways);

  for (uint i = 0u; i < numTextures; ++i)
  {
    ImageData& data = myImageDatas[i];
    const TextureProperties& texProps = data.myTexture->GetProperties();

    ImGui::Checkbox(data.myName.c_str(), &data.myIsWindowOpen);
    if (data.myIsWindowOpen)
    {
      ImGui::SetNextWindowSize(ImVec2(static_cast<float>(texProps.myWidth * 2), static_cast<float>(texProps.myHeight * 2)));
      ImGui::Begin(data.myName.c_str());
      ImGui::SliderInt("Mip Level", &data.mySelectedMipLevel, 0, texProps.myNumMipLevels - 1);
      data.myIsDirty |= ImGui::ListBox("Downsample Filter", &data.mySelectedFilter, locResampleFilterNames, ARRAY_LENGTH(locResampleFilterNames));

      if (data.myIsDirty | myUpdateAlways)
      {
        myAssetManager->ComputeMipmaps(data.myTexture, (AssetManager::ResampleFilter) data.mySelectedFilter);
        data.myIsDirty = false;
      }

      ImGui::Image((ImTextureID)data.myMipLevelReadViews[data.mySelectedMipLevel].get(), ImVec2(static_cast<float>(texProps.myWidth), static_cast<float>(texProps.myHeight)));
      ImGui::End();
    }
  }
}

void Test_Mipmapping::OnShaderRecompiled(const Fancy::GpuProgram* aShader)
{
  if (aShader == myAssetManager->GetResizeShader())
  {
    for (ImageData& data : myImageDatas)
      data.myIsDirty = true;
  }
}


