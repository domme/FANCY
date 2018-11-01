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

struct MipmapData
{
  MipmapData(SharedPtr<Texture> aTexture) { Create(aTexture); }
  MipmapData() {}
  void Create(SharedPtr<Texture> aTexture);
  void Clear() { myTexture.reset(); myTextureView.reset(); myMipLevelReadViews.clear(); myMipLevelWriteViews.clear(); }
  bool myIsSRGB = false;
  SharedPtr<Texture> myTexture;
  SharedPtr<TextureView> myTextureView;
  DynamicArray<SharedPtr<TextureView>> myMipLevelReadViews;
  DynamicArray<SharedPtr<TextureView>> myMipLevelWriteViews;
};

void MipmapData::Create(SharedPtr<Texture> aTexture)
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
}

struct GUIstate
{
  DynamicArray<String> textureNames;
  DynamicArray<bool> windowsOpened;
  DynamicArray<int> selectedMipLevels;
  DynamicArray<int> selectedFilters;
};
  
FancyRuntime* myRuntime = nullptr;
Window* myWindow = nullptr;
RenderOutput* myRenderOutput = nullptr;
AssetStorage myAssetStorage;
GUIstate myGuiState;

SharedPtr<GpuProgramPipeline> myTexturedQuadShader;

DynamicArray<MipmapData> myMipmapData;

InputState myInputState;
int mySelectedMipLevel = 0;

enum DownsampleFilter
{
  FILTER_LINEAR = 0,
  FILTER_LANCZOS,

  FILTER_NUM
};
const char* myDownsampleFilterNames[] = { "Linear", "Lanczos" };
SharedPtr<GpuProgram> myDownsampleTextureShader[FILTER_NUM];

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

  myTexturedQuadShader = LoadShader("Unlit_Quad", "main", "main_textured");
  ASSERT(myTexturedQuadShader != nullptr);

  GpuProgramDesc shaderDesc;
  shaderDesc.myShaderFileName = "DownsampleCS";
  shaderDesc.myShaderStage = (uint) ShaderStage::COMPUTE;
  shaderDesc.myMainFunction = "main_linear";
  myDownsampleTextureShader[FILTER_LINEAR] =  RenderCore::CreateGpuProgram(shaderDesc);
  ASSERT(myDownsampleTextureShader[FILTER_LINEAR]);

  shaderDesc.myMainFunction = "main_lanczos";
  myDownsampleTextureShader[FILTER_LANCZOS] =  RenderCore::CreateGpuProgram(shaderDesc);
  ASSERT(myDownsampleTextureShader[FILTER_LANCZOS]);

  const uint loadFlags = AssetStorage::NO_DISK_CACHE | AssetStorage::NO_MEM_CACHE | AssetStorage::SHADER_WRITABLE;
  myMipmapData.push_back(myAssetStorage.CreateTexture("Textures/Checkerboard.png", loadFlags));
  myMipmapData.push_back(myAssetStorage.CreateTexture("Textures/Sibenik/kamen.png", loadFlags));
  myMipmapData.push_back(myAssetStorage.CreateTexture("Textures/Sibenik/mramor6x6.png", loadFlags));

  const uint numTextures = myMipmapData.size();
  myGuiState.textureNames.resize(numTextures);
  myGuiState.windowsOpened.resize(numTextures);
  myGuiState.selectedMipLevels.resize(numTextures);
  myGuiState.selectedFilters.resize(numTextures);

  for (uint i = 0u; i < numTextures; ++i)
  {
    String texturePath = myMipmapData[i].myTexture->GetProperties().path;
    String texName = texturePath.substr(texturePath.find_last_of('/') + 1);

    myGuiState.textureNames[i] = texName;
    myGuiState.windowsOpened[i] = false;
    myGuiState.selectedMipLevels[i] = 0;
    myGuiState.selectedFilters[i] = 0;
  }

  ImGuiRendering::Init(myRuntime->GetRenderOutput(), myRuntime);
}

void UpdateGUI()
{
  const uint numTextures = myMipmapData.size();

  for (uint i = 0u; i < numTextures; ++i)
  {
    MipmapData& data = myMipmapData[i];
    const TextureProperties& texProps = data.myTexture->GetProperties();

    bool windowOpen = myGuiState.windowsOpened[i];
    ImGui::Checkbox(myGuiState.textureNames[i].c_str(), &windowOpen);
    myGuiState.windowsOpened[i] = windowOpen;

    if (windowOpen)
    {
      ImGui::Begin(myGuiState.textureNames[i].c_str());
      ImGui::SliderInt("Mip Level", &myGuiState.selectedMipLevels[i], 0, texProps.myNumMipLevels - 1);
      ImGui::ListBox("Downsample Filter", &myGuiState.selectedFilters[i], myDownsampleFilterNames, ARRAY_LENGTH(myDownsampleFilterNames));
      ImGui::Image((ImTextureID) data.myMipLevelReadViews[myGuiState.selectedMipLevels[i]].get(), ImVec2(texProps.myWidth, texProps.myHeight));
      ImGui::End();
    }
  }
}

void Update()
{
  myRuntime->BeginFrame();
  ImGuiRendering::NewFrame();
  const float deltaTime = 0.016f;
  myRuntime->Update(deltaTime);

  UpdateGUI();
}

void ComputeMipMaps(MipmapData& aMipmapData, const GpuProgram* aDownsampleShader)
{
    CommandQueue* queue = RenderCore::GetCommandQueue(CommandListType::Graphics);
    CommandContext* ctx = RenderCore::AllocateContext(CommandListType::Graphics);

    ctx->SetComputeProgram(aDownsampleShader);

    uint numMips = aMipmapData.myTexture->GetProperties().myNumMipLevels;

    glm::float2 size(aMipmapData.myTexture->GetProperties().myWidth, aMipmapData.myTexture->GetProperties().myHeight);
    size = glm::ceil(size * 0.5f);

    uint mip = 1u;
    for (; mip < numMips && size.x > 0 && size.y > 0; ++mip)
    {
      struct CBuffer
      {
        glm::float2 mySizeOnMipInv;
        int myMip;
        int myIsSRGB;
      };
      CBuffer cBuffer = 
      {
        glm::float2(1.0f / size.x, 1.0f / size.y),
        (int) mip,
        aMipmapData.myIsSRGB ? 1 : 0
      };
      ctx->BindConstantBuffer(&cBuffer, sizeof(cBuffer), 0u);

      const GpuResourceView* resources[] = { aMipmapData.myMipLevelReadViews[mip-1].get(), aMipmapData.myMipLevelWriteViews[mip].get()};
      ctx->BindResourceSet(resources, 2, 1u);

      ctx->Dispatch((uint) size.x, (uint) size.y, 1);
     
      size = glm::ceil(size * 0.5f);
    }

    queue->ExecuteContext(ctx, true);
    RenderCore::FreeContext(ctx); 
}

void Render()
{
  for (uint i = 0; i < myMipmapData.size(); ++i)
  {
    if (myGuiState.windowsOpened[i])
      ComputeMipMaps(myMipmapData[i], myDownsampleTextureShader[myGuiState.selectedFilters[i]].get());  
  }

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
  for (SharedPtr<GpuProgram>& shader : myDownsampleTextureShader)
    shader.reset();
  
  myTexturedQuadShader.reset();
  myMipmapData.clear();
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