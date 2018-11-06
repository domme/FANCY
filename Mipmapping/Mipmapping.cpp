#include <windows.h>
#include <fancy_core/RenderOutput.h>
#include <fancy_core/RenderCore.h>
#include <fancy_core/RendererPrerequisites.h>
#include <fancy_core/CommandListType.h>
#include "fancy_core/CommandContext.h"
#include <fancy_core/Fancy.h>
#include <fancy_core/Window.h>
#include <fancy_core/GpuProgramPipelineDesc.h>
#include <fancy_core/Descriptor.h>
#include <fancy_core/CommandQueue.h>

#include <fancy_imgui/imgui.h>
#include <fancy_imgui/imgui_impl_fancy.h>

#include "fancy_assets/Model.h"
#include "fancy_assets/Material.h"
#include "fancy_core/Mesh.h"
#include <fancy_core/Texture.h>
#include <fancy_core/Input.h>
#include "fancy_assets/AssetStorage.h"

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
  SharedPtr<Texture> myResampledTexture;
  SharedPtr<TextureView> myResampledTextureView;
  DynamicArray<SharedPtr<TextureView>> myMipLevelReadViews;
  DynamicArray<SharedPtr<TextureView>> myMipLevelWriteViews;

  String myName;
  bool myIsWindowOpen;
  bool myShowMip;
  bool myShowRescaled;
  bool myIsDirty;
  int mySelectedMipLevel;
  int mySelectedFilter;
  float myScalingFactors[2];
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
  myShowMip = true;
  myShowRescaled = false;
  myIsDirty = true;
  mySelectedMipLevel = 0;
  mySelectedFilter = FILTER_LINEAR;
  myScalingFactors[0] = 0.5f;
  myScalingFactors[1] = 0.5f;
}

FancyRuntime* myRuntime = nullptr;
Window* myWindow = nullptr;
RenderOutput* myRenderOutput = nullptr;
AssetStorage myAssetStorage;

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

void Rescale(ImageData& aMipmapData, float scaleX, float scaleY)
{
  const TextureProperties& srcTexProps = aMipmapData.myTexture->GetProperties();
  
  TextureProperties destTexProps = srcTexProps;
  destTexProps.myWidth = (int) glm::ceil((float) srcTexProps.myWidth * scaleX);
  destTexProps.myHeight = (int) glm::ceil((float) srcTexProps.myHeight * scaleY);

  if (destTexProps.myWidth == 0 || destTexProps.myHeight == 0)
    return;

  TextureViewProperties destTexViewProps = aMipmapData.myTextureView->GetProperties();

  if (aMipmapData.myResampledTexture == nullptr || aMipmapData.myResampledTexture->GetProperties().myWidth != destTexProps.myWidth || aMipmapData.myResampledTexture->GetProperties().myHeight != destTexProps.myHeight)
  {
    String name = aMipmapData.myTexture->myName + "_rescaled";
    destTexProps.myNumMipLevels = 1;
    aMipmapData.myResampledTexture = RenderCore::CreateTexture(destTexProps, name.c_str());
    aMipmapData.myResampledTextureView = RenderCore::CreateTextureView(aMipmapData.myResampledTexture, destTexViewProps);  
  }
    
  destTexViewProps.myFormat = Fancy::DataFormatInfo::GetNonSRGBformat(destTexProps.eFormat);
  destTexViewProps.myIsShaderWritable = true;
  SharedPtr<TextureView> resampledTexWriteView = RenderCore::CreateTextureView(aMipmapData.myResampledTexture, destTexViewProps);
  
  CommandQueue* queue = RenderCore::GetCommandQueue(CommandListType::Graphics);
  CommandContext* ctx = RenderCore::AllocateContext(CommandListType::Graphics);
  ctx->SetComputeProgram(myResampleShader.get());

  struct CBuffer
  {
    glm::float2 mySrcSize;
    glm::float2 myDestSize;
    int myIsSRGB;
    int myFilterMethod;
  };
  CBuffer cBuffer = 
  {
    glm::float2(srcTexProps.myWidth, srcTexProps.myHeight),
    glm::float2(destTexProps.myWidth, destTexProps.myHeight),
    aMipmapData.myIsSRGB ? 1 : 0,
    aMipmapData.mySelectedFilter
  };
  ctx->BindConstantBuffer(&cBuffer, sizeof(cBuffer), 0u);

  const GpuResourceView* resources[] = { aMipmapData.myTextureView.get(), resampledTexWriteView.get()};
  ctx->BindResourceSet(resources, 2, 1u);

  ctx->Dispatch((uint) destTexProps.myWidth, (uint) destTexProps.myHeight, 1);

  queue->ExecuteContext(ctx, true);
  RenderCore::FreeContext(ctx); 
}

void ComputeMipMaps(ImageData& aMipmapData)
{
    CommandQueue* queue = RenderCore::GetCommandQueue(CommandListType::Graphics);
    CommandContext* ctx = RenderCore::AllocateContext(CommandListType::Graphics);
    ctx->SetComputeProgram(myResampleShader.get());

    const uint numMips = aMipmapData.myTexture->GetProperties().myNumMipLevels;
    glm::float2 srcSize(aMipmapData.myTexture->GetProperties().myWidth, aMipmapData.myTexture->GetProperties().myHeight);
    glm::float2 destSize = glm::ceil(srcSize * 0.5f);

    uint mip = 1u;
    for (; mip < numMips; ++mip)
    {
      struct CBuffer
      {
        glm::float2 mySrcSize;
        glm::float2 myDestSize;
        int myIsSRGB;
        int myFilterMethod;
      };
      CBuffer cBuffer = 
      {
        srcSize,
        destSize,
        aMipmapData.myIsSRGB ? 1 : 0,
        aMipmapData.mySelectedFilter
      };
      ctx->BindConstantBuffer(&cBuffer, sizeof(cBuffer), 0u);

      const GpuResourceView* resources[] = { aMipmapData.myMipLevelReadViews[mip-1].get(), aMipmapData.myMipLevelWriteViews[mip].get()};
      ctx->BindResourceSet(resources, 2, 1u);

      ctx->Dispatch((uint) destSize.x, (uint) destSize.y, 1);
     
      srcSize *= 0.5f;
      destSize *= 0.5f;
    }

    queue->ExecuteContext(ctx, true);
    RenderCore::FreeContext(ctx); 
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
  shaderDesc.myShaderFileName = "DownsampleCS";
  shaderDesc.myShaderStage = (uint) ShaderStage::COMPUTE;
  shaderDesc.myMainFunction = "main";
  myResampleShader =  RenderCore::CreateGpuProgram(shaderDesc);
  ASSERT(myResampleShader);

  const uint loadFlags = AssetStorage::NO_DISK_CACHE | AssetStorage::NO_MEM_CACHE | AssetStorage::SHADER_WRITABLE;
  myImageDatas.push_back(myAssetStorage.CreateTexture("Textures/Checkerboard.png", loadFlags));
  myImageDatas.push_back(myAssetStorage.CreateTexture("Textures/Sibenik/kamen.png", loadFlags));
  myImageDatas.push_back(myAssetStorage.CreateTexture("Textures/Sibenik/mramor6x6.png", loadFlags));

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
  static bool isFirstUpdate = true;

  myRuntime->BeginFrame();
  ImGuiRendering::NewFrame();
  const float deltaTime = 0.016f;
  myRuntime->Update(deltaTime);

  const uint numTextures = myImageDatas.size();
  for (uint i = 0u; i < numTextures; ++i)
  {
    ImageData& data = myImageDatas[i];
    const TextureProperties& texProps = data.myTexture->GetProperties();
    
    ImGui::Checkbox(data.myName.c_str(), &data.myIsWindowOpen);
    if (data.myIsWindowOpen)
    {
      ImGui::SetNextWindowSize(ImVec2(texProps.myWidth * 2, texProps.myHeight * 2));
      ImGui::Begin(data.myName.c_str());
      ImGui::SliderInt("Mip Level", &data.mySelectedMipLevel, 0, texProps.myNumMipLevels - 1);
      bool scaleChanged = ImGui::InputFloat("Scale", &data.myScalingFactors[0]);
      data.myScalingFactors[1] = data.myScalingFactors[0];
      //ImGui::SliderFloat2("Resize", data.myScalingFactors, 0.01f, 10.0f);
      ImGui::ListBox("Downsample Filter", &data.mySelectedFilter, myResampleFilterNames, ARRAY_LENGTH(myResampleFilterNames));
      ImGui::Checkbox("Show Mip", &data.myShowMip);
      ImGui::Checkbox("Show Rescale", &data.myShowRescaled);

      if (data.myIsDirty)
      {
        ComputeMipMaps(data);
        Rescale(data, data.myScalingFactors[0], data.myScalingFactors[1]);
        data.myIsDirty = false;
      }
      else if (scaleChanged)
        Rescale(data, data.myScalingFactors[0], data.myScalingFactors[1]);

      if (data.myShowMip)
        ImGui::Image((ImTextureID) data.myMipLevelReadViews[data.mySelectedMipLevel].get(), ImVec2(texProps.myWidth, texProps.myHeight));
      if (data.myShowRescaled)
        ImGui::Image((ImTextureID) data.myResampledTextureView.get(), ImVec2(texProps.myWidth, texProps.myHeight));
      
      ImGui::End();
    }
  }

  isFirstUpdate = false;
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
  myAssetStorage.Clear();

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