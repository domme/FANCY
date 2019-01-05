#include <windows.h>
#include <fancy_core/RenderOutput.h>
#include <fancy_core/RenderCore.h>
#include <fancy_core/RendererPrerequisites.h>
#include "fancy_core/CommandContext.h"
#include <fancy_core/Fancy.h>
#include <fancy_core/Window.h>
#include <fancy_core/GpuProgramPipelineDesc.h>
#include <fancy_core/CommandQueue.h>

#include <fancy_imgui/imgui.h>
#include <fancy_imgui/imgui_impl_fancy.h>

#include <fancy_core/Texture.h>
#include <fancy_core/Input.h>
#include "fancy_assets/AssetManager.h"
#include <fancy_core/Log.h>

using namespace Fancy;

enum ResampleFilter
{
  FILTER_LINEAR = 0,
  FILTER_LANCZOS,

  FILTER_NUM
};

struct ImageData
{
  ImageData(SharedPtr<Texture> aTexture) { Create(aTexture); }
  ImageData() {}
  void Create(SharedPtr<Texture> aTexture);
  void Clear() { myTexture.reset(); myTextureView.reset(); myMipLevelReadViews.clear(); myMipLevelWriteViews.clear(); }
  bool myIsSRGB = false;

  SharedPtr<Texture> myTexture;
  SharedPtr<TextureView> myTextureView;
  DynamicArray<SharedPtr<TextureView>> myMipLevelReadViews;
  DynamicArray<SharedPtr<TextureView>> myMipLevelWriteViews;

  String myName;
  bool myIsWindowOpen;
  bool myIsDirty;
  int mySelectedMipLevel;
  int mySelectedFilter;
};

void ImageData::Create(SharedPtr<Texture> aTexture)
{
  const TextureProperties& destTexProps = aTexture->GetProperties();
  if(destTexProps.myNumMipLevels == 1)
    return;

  myTexture = aTexture;
  TextureViewProperties readProps;
  readProps.myFormat = aTexture->GetProperties().eFormat;
  readProps.myDimension = GpuResourceDimension::TEXTURE_2D;
  myTextureView = RenderCore::CreateTextureView(aTexture, readProps);
  ASSERT(myTextureView != nullptr);

  const DataFormatInfo& destTexFormatInfo = DataFormatInfo::GetFormatInfo(destTexProps.eFormat);
  myIsSRGB = destTexFormatInfo.mySRGB;
  
  readProps.myNumMipLevels = 1;

  TextureViewProperties writeProps = readProps;
  writeProps.myFormat = DataFormatInfo::GetNonSRGBformat(readProps.myFormat);
  writeProps.myIsShaderWritable = true;

  const uint numMips = destTexProps.myNumMipLevels;
  myMipLevelReadViews.resize(numMips);
  myMipLevelWriteViews.resize(numMips);
  
  for (uint mip = 0u; mip < numMips; ++mip)
  {
    readProps.myMipIndex = mip;
    writeProps.myMipIndex = mip;
    myMipLevelReadViews[mip] = RenderCore::CreateTextureView(aTexture, readProps);
    myMipLevelWriteViews[mip] = RenderCore::CreateTextureView(aTexture, writeProps);
    ASSERT(myMipLevelReadViews[mip] != nullptr && myMipLevelWriteViews[mip] != nullptr);
  }

  String texturePath = destTexProps.path;
  myName = texturePath.substr(texturePath.find_last_of('/') + 1);
  myIsWindowOpen = false;
  myIsDirty = false;
  mySelectedMipLevel = 0;
  mySelectedFilter = FILTER_LINEAR;
}

FancyRuntime* myRuntime = nullptr;
Window* myWindow = nullptr;
RenderOutput* myRenderOutput = nullptr;
SharedPtr<AssetManager> myAssetManager;

DynamicArray<ImageData> myImageDatas;

InputState myInputState;
int mySelectedMipLevel = 0;

const char* myResampleFilterNames[] = { "Linear", "Lanczos" };
SharedPtr<GpuProgram> myResampleShader;

void OnShaderRecompiled(const GpuProgram* aShader);

void OnWindowResized(uint aWidth, uint aHeight)
{
  // myCamera.myWidth = myWindow->GetWidth();
  // myCamera.myHeight = myWindow->GetHeight();
  // myCamera.UpdateProjection();
}

SharedPtr<GpuProgramPipeline> LoadShader(const char* aShaderPath, const char* aMainVtxFunction = "main", const char* aMainFragmentFunction = "main")
{
  GpuProgramPipelineDesc pipelineDesc;
  GpuProgramDesc* shaderDesc = &pipelineDesc.myGpuPrograms[(uint)ShaderStage::VERTEX];
  shaderDesc->myShaderFileName = aShaderPath;
  shaderDesc->myMainFunction = aMainVtxFunction;
  shaderDesc = &pipelineDesc.myGpuPrograms[(uint)ShaderStage::FRAGMENT];
  shaderDesc->myShaderFileName = aShaderPath;
  shaderDesc->myMainFunction = aMainFragmentFunction;
  return RenderCore::CreateGpuProgramPipeline(pipelineDesc);
}

void ComputeMipMaps(ImageData& aMipmapData)
{
  myAssetManager->ComputeMipmaps(aMipmapData.myTexture);
}

void Init(HINSTANCE anInstanceHandle)
{
  Fancy::RenderingStartupParameters params;
  params.myRenderingTechnique = RenderingTechnique::FORWARD;

  Fancy::WindowParameters windowParams;
  windowParams.myWidth = 1280;
  windowParams.myHeight = 720;
  windowParams.myTitle = "Mipmapping Test";

  myRuntime = FancyRuntime::Init(anInstanceHandle, params, windowParams);

  myRenderOutput = myRuntime->GetRenderOutput();
  myWindow = myRenderOutput->GetWindow();

  std::function<void(uint, uint)> onWindowResized = &OnWindowResized;
  myWindow->myOnResize.Connect(onWindowResized);
  myWindow->myWindowEventHandler.Connect(&myInputState, &InputState::OnWindowEvent);

  GpuProgramDesc shaderDesc;
  shaderDesc.myShaderFileName = "ResizeTexture2D";
  shaderDesc.myShaderStage = (uint) ShaderStage::COMPUTE;
  shaderDesc.myMainFunction = "main";
  myResampleShader =  RenderCore::CreateGpuProgram(shaderDesc);
  ASSERT(myResampleShader);

  myAssetManager.reset(new AssetManager());

  const uint loadFlags = AssetManager::SHADER_WRITABLE;
  myImageDatas.push_back(myAssetManager->CreateTexture("Textures/Sibenik/kamen.png", loadFlags));
  myImageDatas.push_back(myAssetManager->CreateTexture("Textures/Checkerboard.png", loadFlags));
  myImageDatas.push_back(myAssetManager->CreateTexture("Textures/Sibenik/mramor6x6.png", loadFlags));

  ImGuiRendering::Init(myRuntime->GetRenderOutput(), myRuntime);

  std::function<void(const GpuProgram*)> onShaderRecompiledFn = &OnShaderRecompiled;
  RenderCore::ourOnShaderRecompiled.Connect(onShaderRecompiledFn);
}

void OnShaderRecompiled(const GpuProgram* aShader)
{
  if (aShader == myResampleShader.get())
  {
    for (ImageData& data : myImageDatas)
      data.myIsDirty = true;
  }
}

void Update()
{
  myRuntime->BeginFrame();
  ImGuiRendering::NewFrame();
  const float deltaTime = 0.016f;
  myRuntime->Update(deltaTime);

  const uint numTextures = (uint) myImageDatas.size();

  static bool updateAlways = false;
  ImGui::Checkbox("Update every frame", &updateAlways);

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
      data.myIsDirty |= ImGui::ListBox("Downsample Filter", &data.mySelectedFilter, myResampleFilterNames, ARRAY_LENGTH(myResampleFilterNames));

      if (data.myIsDirty | updateAlways)
      {
        ComputeMipMaps(data);
        data.myIsDirty = false;
      }

      ImGui::Image((ImTextureID) data.myMipLevelReadViews[data.mySelectedMipLevel].get(), ImVec2(static_cast<float>(texProps.myWidth), static_cast<float>(texProps.myHeight)));

      ImGui::End();
    }
  }
}

void Render()
{
  CommandQueue* queue = RenderCore::GetCommandQueue(CommandListType::Graphics);
  CommandContext* ctx = RenderCore::AllocateContext(CommandListType::Graphics);
  float clearColor[] = { 0.3f, 0.3f, 0.3f, 0.0f };
  ctx->ClearRenderTarget(myRenderOutput->GetBackbufferRtv(), clearColor);
  ctx->ClearDepthStencilTarget(myRenderOutput->GetDepthStencilDsv(), 1.0f, 0u);
  queue->ExecuteContext(ctx);
  RenderCore::FreeContext(ctx);
  
  ImGui::Render();

  myRuntime->EndFrame();
}

void Shutdown()
{
  myResampleShader.reset();
  
  myImageDatas.clear();
  myAssetManager.reset();

  ImGuiRendering::Shutdown();
  FancyRuntime::Shutdown();
  myRuntime = nullptr;
}

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
  Init(hInstance);

  MSG msg = { 0 };
  while (true)
  {
    // Process any messages in the queue.
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    
      if (msg.message == WM_QUIT)
        break;
    }

    Update();
    Render();
  }

  Shutdown();

  return 0;
}