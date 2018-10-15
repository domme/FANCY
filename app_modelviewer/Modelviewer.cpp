#include <windows.h>
#include <fancy_core/RenderOutput.h>
#include <fancy_core/RenderCore.h>
#include <fancy_core/RendererPrerequisites.h>
#include <fancy_core/CommandListType.h>
#include "fancy_core/CommandContext.h"
#include <fancy_core/Fancy.h>
#include <fancy_core/Window.h>
#include <fancy_core/GpuProgramPipelineDesc.h>
#include <fancy_core/GpuBuffer.h>
#include <fancy_core/Descriptor.h>
#include <fancy_core/CommandQueue.h>

#include <fancy_assets/ModelLoader.h>
#include <fancy_assets/AssetStorage.h>
#include "Camera.h"
#include "fancy_assets/Model.h"
#include "fancy_assets/Material.h"
#include "fancy_core/Mesh.h"
#include <fancy_core/Texture.h>
#include <fancy_core/Input.h>
#include "CameraController.h"
#include <fancy_core/MeshData.h>
#include <fancy_core/ResourceRefs.h>

using namespace Fancy;

FancyRuntime* myRuntime = nullptr;
Window* myWindow = nullptr;
RenderOutput* myRenderOutput = nullptr;
ModelLoader::Scene myScene;
AssetStorage myAssetStorage;

SharedPtr<GpuProgramPipeline> myUnlitTexturedShader;
SharedPtr<GpuProgramPipeline> myUnlitVertexColorShader;
SharedPtr<GpuProgramPipeline> myDebugGeoShader;
SharedPtr<CameraController> myCameraController;

Camera myCamera;
InputState myInputState;

void OnWindowResized(uint aWidth, uint aHeight)
{
  myCamera.myWidth = myWindow->GetWidth();
  myCamera.myHeight = myWindow->GetHeight();
  myCamera.UpdateProjection();
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

  myRuntime = FancyRuntime::Init(anInstanceHandle, params);

  myRenderOutput = myRuntime->GetRenderOutput();
  myWindow = myRenderOutput->GetWindow();

  std::function<void(uint, uint)> onWindowResized = &OnWindowResized;
  myWindow->myOnResize.Connect(onWindowResized);
  myWindow->myWindowEventHandler.Connect(&myInputState, &InputState::OnWindowEvent);

  myUnlitTexturedShader = LoadShader("Unlit_Textured");
  ASSERT(myUnlitTexturedShader != nullptr);

  myUnlitVertexColorShader = LoadShader("Unlit_Colored");
  ASSERT(myUnlitVertexColorShader != nullptr);

  myDebugGeoShader = LoadShader("DebugGeo_Colored");
  ASSERT(myDebugGeoShader != nullptr);

  myCamera.myPosition = glm::float3(0.0f, 0.0f, -10.0f);
  myCamera.myOrientation = glm::quat_cast(glm::lookAt(glm::float3(0.0f, 0.0f, 10.0f), glm::float3(0.0f, 0.0f, 0.0f), glm::float3(0.0f, 1.0f, 0.0f)));

  myCamera.myFovDeg = 60.0f;
  myCamera.myNear = 1.0f;
  myCamera.myFar = 100.0f;
  myCamera.myWidth = myWindow->GetWidth();
  myCamera.myHeight = myWindow->GetHeight();
  myCamera.myIsOrtho = false;

  myCamera.UpdateView();
  myCamera.UpdateProjection();

  myCameraController.reset(new CameraController(myWindow, &myCamera));

  bool importSuccess = ModelLoader::LoadFromFile("models/cube.obj", myAssetStorage, myScene);
  ASSERT(importSuccess);
}

void Update()
{
  myRuntime->BeginFrame();

  const float deltaTime = 0.016f;
  myRuntime->Update(deltaTime);
  myCameraController->Update(deltaTime, myInputState);
}

void BindResources_UnlitTextured(CommandContext* aContext, Material* aMat)
{
  const GpuResourceView* diffuseTex = aMat->mySemanticTextures[(uint)TextureSemantic::BASE_COLOR].get();
  if (diffuseTex)
  {
    aContext->BindResourceSet(&diffuseTex, 1u, 1u);
  }
}

void RenderGrid(CommandContext* ctx)
{
  ctx->SetViewport(glm::uvec4(0, 0, myWindow->GetWidth(), myWindow->GetHeight()));
  ctx->SetClipRect(glm::uvec4(0, 0, myWindow->GetWidth(), myWindow->GetHeight()));
  ctx->SetRenderTarget(myRenderOutput->GetBackbufferRtv(), myRenderOutput->GetDepthStencilDsv());
  
  ctx->SetDepthStencilState(nullptr);
  ctx->SetBlendState(nullptr);
  ctx->SetCullMode(CullMode::NONE);
  ctx->SetFillMode(FillMode::SOLID);
  ctx->SetWindingOrder(WindingOrder::CCW);
  
  ctx->SetGpuProgramPipeline(myDebugGeoShader);
  
  struct Cbuffer_DebugGeo
  {
    glm::float4x4 myWorldViewProj;
    glm::float4 myColor;
  };
  Cbuffer_DebugGeo cbuffer_debugGeo
  {
    myCamera.myViewProj,
    glm::float4(1.0f, 0.0f, 0.0f, 1.0f),
  };
  ctx->BindConstantBuffer(&cbuffer_debugGeo, sizeof(cbuffer_debugGeo), 0u);

  struct GridGeoVertex
  {
    glm::float3 myPos;
    glm::u8vec4 myColor;
  };

  GridGeoVertex vertices[4] = {
    { { 0.0f, 0.0f, -1.0f }, {0,0,255,255} },
    { { 0.0f, 0.0f, 1.0f }, {0.0f, 0.0f, 1.0f, 1.0f} },
    { { -1.0f, 0.0f, 0.0f }, {1.0f, 0.0f, 0.0f, 1.0f } },
    { { 1.0f, 0.0f, 0.0f },{1.0f, 0.0f, 0.0f, 1.0f } }
  };
  ctx->BindVertexBuffer(vertices, sizeof(vertices), sizeof(vertices[0]));

  uint indices[] = {
    0, 1, 2, 3
  };
  ctx->BindIndexBuffer(indices, sizeof(indices), sizeof(indices[0]));

  ctx->SetTopologyType(TopologyType::LINES);
  ctx->Render(4, 1, 0, 0, 0);
}

void RenderScene(CommandContext* ctx)
{
  ctx->SetViewport(glm::uvec4(0, 0, myWindow->GetWidth(), myWindow->GetHeight()));
  ctx->SetClipRect(glm::uvec4(0, 0, myWindow->GetWidth(), myWindow->GetHeight()));
  ctx->SetRenderTarget(myRenderOutput->GetBackbufferRtv(), myRenderOutput->GetDepthStencilDsv());
  
  ctx->SetDepthStencilState(nullptr);
  ctx->SetBlendState(nullptr);
  ctx->SetCullMode(CullMode::NONE);
  ctx->SetFillMode(FillMode::SOLID);
  ctx->SetWindingOrder(WindingOrder::CCW);

  ctx->SetTopologyType(TopologyType::TRIANGLE_LIST);
  ctx->SetGpuProgramPipeline(myUnlitTexturedShader);
  for (int i = 0; i < myScene.myModels.size(); ++i)
  {
    Model* model = myScene.myModels[i].get();
    
    struct Cbuffer_PerObject
    {
      glm::float4x4 myWorldViewProj;
    };
    Cbuffer_PerObject cbuffer_perObject
    {
      myCamera.myViewProj * myScene.myTransforms[i],
    };
    ctx->BindConstantBuffer(&cbuffer_perObject, sizeof(cbuffer_perObject), 0u);
    
    Material* mat = model->myMaterial.get();
    BindResources_UnlitTextured(ctx, mat);
    
    Mesh* mesh = model->myMesh.get();
    for (SharedPtr<GeometryData>& geometry : mesh->myGeometryDatas)
      ctx->RenderGeometry(geometry.get());
  }
}

void Render()
{
  CommandQueue* queue = RenderCore::GetCommandQueue(CommandListType::Graphics);
  CommandContext* ctx = RenderCore::AllocateContext(CommandListType::Graphics);
  float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
  ctx->ClearRenderTarget(myRenderOutput->GetBackbufferRtv(), clearColor);
  ctx->ClearDepthStencilTarget(myRenderOutput->GetDepthStencilDsv(), 1.0f, 0u);

  RenderGrid(ctx);
  RenderScene(ctx);  

  queue->ExecuteContext(ctx);
  RenderCore::FreeContext(ctx);

  myRuntime->EndFrame();
}

void Shutdown()
{
  myScene.Clear();
  myUnlitTexturedShader.reset();
  myAssetStorage.Clear();

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