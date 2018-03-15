#include <windows.h>
#include <fancy_core/RenderOutput.h>
#include <fancy_core/RenderCore.h>
#include <fancy_core/RendererPrerequisites.h>
#include <fancy_core/CommandListType.h>
#include "fancy_core/CommandContext.h"
#include <fancy_core/Fancy.h>
#include <fancy_core/Window.h>
#include <fancy_core/GpuProgramPipelineDesc.h>

#include <fancy_assets/ModelLoader.h>
#include <fancy_assets/GraphicsWorld.h>
#include "Camera.h"
#include "fancy_assets/Model.h"
#include "fancy_assets/Material.h"
#include "fancy_core/Mesh.h"

using namespace Fancy;

FancyRuntime* myRuntime = nullptr;
Window* myWindow = nullptr;
ModelLoader::Scene myScene;
UniquePtr<GraphicsWorld> myGraphicsWorld;

SharedPtr<GpuBuffer> myCbufferPerObject;
SharedPtr<GpuProgramPipeline> myUnlitTexturedShader;
Camera myCamera;

struct CBuffer_PerObject
{
  glm::float4x4 myWorldViewProj;
};

void OnWindowResized(uint aWidth, uint aHeight)
{
  myCamera.myWidth = myWindow->GetWidth();
  myCamera.myHeight = myWindow->GetHeight();
  myCamera.UpdateProjection();
}

void Init(HINSTANCE anInstanceHandle)
{
  Fancy::RenderingStartupParameters params;
  params.myRenderingTechnique = RenderingTechnique::FORWARD;

  myRuntime = FancyRuntime::Init(anInstanceHandle, params);

  myWindow = myRuntime->GetRenderOutput()->GetWindow();

  std::function<void(uint, uint)> onWindowResized = &OnWindowResized;
  myWindow->myOnResize.Connect(onWindowResized);

  GpuBufferCreationParams bufferParams;
  bufferParams.uNumElements = 1u;
  bufferParams.uElementSizeBytes = sizeof(CBuffer_PerObject);
  bufferParams.myUsageFlags = static_cast<uint>(GpuBufferUsage::CONSTANT_BUFFER);
  bufferParams.uAccessFlags = (uint)GpuResourceAccessFlags::WRITE
    | (uint)GpuResourceAccessFlags::COHERENT
    | (uint)GpuResourceAccessFlags::DYNAMIC
    | (uint)GpuResourceAccessFlags::PERSISTENT_LOCKABLE;

  CBuffer_PerObject initialPerObjectData;
  myCbufferPerObject = RenderCore::CreateBuffer(bufferParams, &initialPerObjectData);
  ASSERT(myCbufferPerObject != nullptr);

  GpuProgramPipelineDesc pipelineDesc;
  GpuProgramDesc* shaderDesc = &pipelineDesc.myGpuPrograms[(uint)ShaderStage::VERTEX];
  shaderDesc->myShaderFileName = "UnlitColored";
  shaderDesc->myMainFunction = "main";
  shaderDesc = &pipelineDesc.myGpuPrograms[(uint)ShaderStage::FRAGMENT];
  shaderDesc->myShaderFileName = "UnlitColored";
  shaderDesc->myMainFunction = "main";
  myUnlitTexturedShader = RenderCore::CreateGpuProgramPipeline(pipelineDesc);
  ASSERT(myUnlitTexturedShader != nullptr);

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
}

void Update()
{
  myRuntime->BeginFrame();
  myRuntime->Update(0.016f);
}

void Render()
{
  CommandContext* ctx = RenderCore::AllocateContext(CommandListType::Graphics);
  float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
  ctx->ClearRenderTarget(myRuntime->GetRenderOutput()->GetBackbuffer(), clearColor);

  for (int i = 0; i < myScene.myModels.size(); ++i)
  {
    Model* model = myScene.myModels[i].get();
    const glm::mat4& transform = myScene.myTransforms[i];

    CBuffer_PerObject cBuffer;
    cBuffer.myWorldViewProj = myCamera.myViewProj * transform;
    RenderCore::UpdateBufferData(myCbufferPerObject.get(), &cBuffer, sizeof(cBuffer));

    Material* mat = model->myMaterial.get();
    Mesh* mesh = model->myMesh.get();

    

    for (SharedPtr<GeometryData>& geometry : mesh->myGeometryDatas)
      ctx->RenderGeometry(geometry.get());
  }

  ctx->ExecuteAndReset();
  RenderCore::FreeContext(ctx);

  myRuntime->EndFrame();
}

void Shutdown()
{
  myCbufferPerObject.reset();
  myGraphicsWorld.reset();
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