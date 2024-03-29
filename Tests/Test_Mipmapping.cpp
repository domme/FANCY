#include "Test_Mipmapping.h"
#include "Rendering/TextureProperties.h"
#include "Rendering/Texture.h"
#include "imgui.h"
#include "Common/Ptr.h"
#include "Debug/Log.h"
#include "IO/AssetManager.h"
#include "Rendering/RenderCore.h"
#include "Rendering/TextureProperties.h"

using namespace Fancy;

static const char* locResampleFilterNames[] = { "Linear", "Lanczos" };

void ImageData::Create(SharedPtr<TextureView> aTexture)
{
  const TextureProperties& destTexProps = aTexture->GetTexture()->GetProperties();
  if (destTexProps.myNumMipLevels == 1)
    return;

  myTextureView = aTexture;
  myTexture = aTexture->GetTexturePtr();
  TextureViewProperties readProps;
  readProps.myFormat = aTexture->GetProperties().myFormat;
  readProps.myDimension = GpuResourceDimension::TEXTURE_2D;
  ASSERT(myTextureView != nullptr);

  const DataFormatInfo& destTexFormatInfo = DataFormatInfo::GetFormatInfo(destTexProps.myFormat);
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
    myMipLevelReadViews[mip] = RenderCore::CreateTextureView(myTexture, readProps);
    myMipLevelWriteViews[mip] = RenderCore::CreateTextureView(myTexture, writeProps);
    ASSERT(myMipLevelReadViews[mip] != nullptr && myMipLevelWriteViews[mip] != nullptr);
  }

  eastl::string texturePath = destTexProps.myPath;
  myName = texturePath.substr(texturePath.find_last_of('/') + 1);
  myIsWindowOpen = false;
  myIsDirty = false;
  mySelectedMipLevel = 0;
  mySelectedFilter = AssetManager::FILTER_LINEAR;
}

Test_Mipmapping::Test_Mipmapping(Fancy::AssetManager* anAssetManager, Fancy::Window* aWindow,
  Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState)
  : Test(anAssetManager, aWindow, aRenderOutput, anInputState, "Mipmapping")
{
  const uint loadFlags = AssetManager::SHADER_WRITABLE;
  myImageDatas.push_back(myAssetManager->LoadTexture("Textures/Sibenik/kamen.png", loadFlags));
  myImageDatas.push_back(myAssetManager->LoadTexture("Textures/Checkerboard.png", loadFlags));
  myImageDatas.push_back(myAssetManager->LoadTexture("Textures/Sibenik/mramor6x6.png", loadFlags));

  RenderCore::ourOnShaderPipelineRecompiled.Connect(this, &Test_Mipmapping::OnShaderPipelineRecompiled);
}

Test_Mipmapping::~Test_Mipmapping()
{
  RenderCore::ourOnShaderPipelineRecompiled.DetachObserver(this);
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

void Test_Mipmapping::OnShaderPipelineRecompiled(const Fancy::ShaderPipeline* aShader)
{
  if (aShader == myAssetManager->GetMipDownsampleShader())
  {
    for (ImageData& data : myImageDatas)
      data.myIsDirty = true;
  }
}
