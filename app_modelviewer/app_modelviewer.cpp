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
#include <fancy_core/TextureRefs.h>

using namespace Fancy;

FancyRuntime* myRuntime = nullptr;
Window* myWindow = nullptr;
RenderOutput* myRenderOutput = nullptr;
ModelLoader::Scene myScene;
AssetStorage myAssetStorage;

SharedPtr<GpuProgramPipeline> myUnlitTexturedShader;
SharedPtr<GpuProgramPipeline> myUnlitVertexColorShader;
SharedPtr<GpuProgramPipeline> myDebugGeoShader;
SharedPtr<GpuBuffer> myCbufferPerObject;
SharedPtr<GpuBuffer> myCbufferDebugGeo;
SharedPtr<CameraController> myCameraController;
SharedPtr<Mesh> myGridMesh;

Camera myCamera;
InputState myInputState;

struct Cbuffer_DebugGeo
{
  glm::float4x4 myWorldViewProj;
  glm::float4 myColor;
};

struct Cbuffer_PerObject
{
  glm::float4x4 myWorldViewProj;
};

void OnWindowResized(uint aWidth, uint aHeight)
{
  myCamera.myWidth = myWindow->GetWidth();
  myCamera.myHeight = myWindow->GetHeight();
  myCamera.UpdateProjection();
}

SharedPtr<GpuProgramPipeline> LoadShader(const char* aShaderPath)
{
  GpuProgramPipelineDesc pipelineDesc;
  GpuProgramDesc* shaderDesc = &pipelineDesc.myGpuPrograms[(uint)ShaderStage::VERTEX];
  shaderDesc->myShaderFileName = aShaderPath;
  shaderDesc->myMainFunction = "main";
  shaderDesc = &pipelineDesc.myGpuPrograms[(uint)ShaderStage::FRAGMENT];
  shaderDesc->myShaderFileName = aShaderPath;
  shaderDesc->myMainFunction = "main";
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

  GpuBufferCreationParams bufferParams;
  bufferParams.uNumElements = 1u;
  bufferParams.uElementSizeBytes = sizeof(Cbuffer_PerObject);
  bufferParams.myUsageFlags = static_cast<uint>(GpuBufferUsage::CONSTANT_BUFFER);
  bufferParams.uAccessFlags = (uint)GpuResourceAccessFlags::WRITE
    | (uint)GpuResourceAccessFlags::COHERENT
    | (uint)GpuResourceAccessFlags::DYNAMIC;

  Cbuffer_PerObject initialPerObjectData;
  myCbufferPerObject = RenderCore::CreateBuffer(bufferParams, &initialPerObjectData);
  ASSERT(myCbufferPerObject != nullptr);

  bufferParams.uElementSizeBytes = sizeof(Cbuffer_DebugGeo);
  Cbuffer_DebugGeo initialDebugGeoData;
  myCbufferDebugGeo = RenderCore::CreateBuffer(bufferParams, &initialDebugGeoData);
  ASSERT(myCbufferDebugGeo != nullptr);

  myUnlitTexturedShader = LoadShader("Unlit_Textured");
  ASSERT(myUnlitTexturedShader != nullptr);

  myUnlitVertexColorShader = LoadShader("Unlit_Colored");
  ASSERT(myUnlitVertexColorShader != nullptr);

  myDebugGeoShader = LoadShader("DebugGeo_Colored");
  ASSERT(myDebugGeoShader != nullptr);

  {
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

     uint indices[] = {
       0, 1, 2, 3
     };

     MeshData meshData;
     meshData.myLayout.myTopology = TopologyType::LINES;
     meshData.myLayout.AddVertexElement(VertexSemantics::POSITION, DataFormat::RGB_32F);
     meshData.myLayout.AddVertexElement(VertexSemantics::COLOR, DataFormat::RGBA_8);
     meshData.myVertexData.resize(sizeof(vertices));
     memcpy(meshData.myVertexData.data(), vertices, sizeof(vertices));
     meshData.myIndexData.resize(sizeof(indices));
     memcpy(meshData.myIndexData.data(), indices, sizeof(indices));

     MeshDesc meshDesc;
     meshDesc.myIsExternalMesh = false;
     meshDesc.myInternalRefIdx = (uint) MeshRef::COORD_GRID;
     myGridMesh = RenderCore::CreateMesh(meshDesc, &meshData, 1u);
     ASSERT(myGridMesh != nullptr);
  }

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
  Texture* diffuseTex = aMat->mySemanticTextures[(uint)TextureSemantic::BASE_COLOR].get();
  if (diffuseTex)
  {
    const Descriptor* desc = diffuseTex->GetDescriptor(DescriptorType::DEFAULT_READ);
    aContext->BindDescriptorSet(&desc, 1u, 1u);
  }
}

void Render()
{
  CommandContext* ctx = RenderCore::AllocateContext(CommandListType::Graphics);
  float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
  ctx->ClearRenderTarget(myRenderOutput->GetBackbuffer(), clearColor);
  ctx->ClearDepthStencilTarget(myRenderOutput->GetDefaultDepthStencilBuffer(), 1.0f, 0u);

  ctx->SetViewport(glm::uvec4(0, 0, myWindow->GetWidth(), myWindow->GetHeight()));
  ctx->SetClipRect(glm::uvec4(0, 0, myWindow->GetWidth(), myWindow->GetHeight()));
  ctx->SetRenderTarget(myRenderOutput->GetBackbuffer(), 0u);
  ctx->SetDepthStencilRenderTarget(myRenderOutput->GetDefaultDepthStencilBuffer());
  
  ctx->SetDepthStencilState(nullptr);
  ctx->SetBlendState(nullptr);
  ctx->SetCullMode(CullMode::NONE);
  ctx->SetFillMode(FillMode::SOLID);
  ctx->SetWindingOrder(WindingOrder::CCW);

  ctx->SetGpuProgramPipeline(myDebugGeoShader);
  Cbuffer_DebugGeo cBufferDebugGeo{ myCamera.myViewProj, glm::float4(1.0f, 0.0f, 0.0f, 1.0f) };
  RenderCore::UpdateBufferData(myCbufferDebugGeo.get(), &cBufferDebugGeo, sizeof(cBufferDebugGeo));
  ctx->BindResource(myCbufferDebugGeo.get(), DescriptorType::CONSTANT_BUFFER, 0u);

  for (SharedPtr<GeometryData>& geometry : myGridMesh->myGeometryDatas)
    ctx->RenderGeometry(geometry.get());

  /*
  ctx->SetGpuProgramPipeline(myUnlitTexturedShader);
  for (int i = 0; i < myScene.myModels.size(); ++i)
  {
    Model* model = myScene.myModels[i].get();
    const glm::mat4& transform = myScene.myTransforms[i];

    Cbuffer_PerObject cBuffer;
    cBuffer.myWorldViewProj = myCamera.myViewProj * transform;
    RenderCore::UpdateBufferData(myCbufferPerObject.get(), &cBuffer, sizeof(cBuffer));
    ctx->BindResource(myCbufferPerObject.get(), DescriptorType::CONSTANT_BUFFER, 0u);

    Material* mat = model->myMaterial.get();
    BindResources_UnlitTextured(ctx, mat);
    
    Mesh* mesh = model->myMesh.get();
    for (SharedPtr<GeometryData>& geometry : mesh->myGeometryDatas)
      ctx->RenderGeometry(geometry.get());
  }
  */

  ctx->ExecuteAndReset();
  RenderCore::FreeContext(ctx);

  myRuntime->EndFrame();
}

void Shutdown()
{
  myCbufferPerObject.reset();
  myUnlitTexturedShader.reset();

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