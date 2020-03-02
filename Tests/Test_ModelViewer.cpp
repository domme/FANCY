#include "Test_ModelViewer.h"
#include "fancy_core/Window.h"
#include "fancy_core/ShaderPipelineDesc.h"
#include "fancy_core/RenderCore.h"
#include "fancy_assets/AssetManager.h"
#include "fancy_core/CommandList.h"
#include "fancy_core/RenderOutput.h"
#include "fancy_assets/Model.h"
#include "fancy_assets/Material.h"
#include "fancy_core/GpuResourceView.h"
#include "fancy_core/Texture.h"
#include "fancy_core/Mesh.h"
#include "fancy_core/TextureSampler.h"
#include "fancy_core/GeometryVertexLayout.h"
#include "fancy_core/GeometryData.h"

using namespace Fancy;

static SharedPtr<ShaderPipeline> locLoadShader(const char* aShaderPath, const char* aMainVtxFunction = "main", const char* aMainFragmentFunction = "main")
{
  ShaderPipelineDesc pipelineDesc;
  ShaderDesc* shaderDesc = &pipelineDesc.myShader[(uint)ShaderStage::VERTEX];
  shaderDesc->myShaderFileName = aShaderPath;
  shaderDesc->myMainFunction = aMainVtxFunction;
  shaderDesc = &pipelineDesc.myShader[(uint)ShaderStage::FRAGMENT];
  shaderDesc->myShaderFileName = aShaderPath;
  shaderDesc->myMainFunction = aMainFragmentFunction;
  return RenderCore::CreateShaderPipeline(pipelineDesc);
}

//---------------------------------------------------------------------------//

Test_ModelViewer::Test_ModelViewer(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow,
  Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState)
  : Test(aRuntime, aWindow, aRenderOutput, anInputState, "Model Viewer")
  , myCameraController(&myCamera)
{
  myAssetManager.reset(new AssetManager());

  myUnlitTexturedShader = locLoadShader("Unlit_Textured");
  ASSERT(myUnlitTexturedShader != nullptr);

  myUnlitVertexColorShader = locLoadShader("Unlit_Colored");
  ASSERT(myUnlitVertexColorShader != nullptr);

  myDebugGeoShader = locLoadShader("DebugGeo_Colored");
  ASSERT(myDebugGeoShader != nullptr);

  TextureSamplerProperties samplerProps;
  samplerProps.myAddressModeX = SamplerAddressMode::REPEAT;
  samplerProps.myAddressModeY = SamplerAddressMode::REPEAT;
  samplerProps.myAddressModeZ = SamplerAddressMode::REPEAT;
  samplerProps.myMinFiltering = SamplerFilterMode::TRILINEAR;
  samplerProps.myMagFiltering = SamplerFilterMode::TRILINEAR;
  mySampler = RenderCore::CreateTextureSampler(samplerProps);

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

  const bool importSuccess = ModelLoader::LoadFromFile("models/cube.obj", *(myAssetManager.get()), myScene);
  ASSERT(importSuccess)
}

Test_ModelViewer::~Test_ModelViewer()
{
  RenderCore::WaitForIdle(CommandListType::Graphics);
}

void Test_ModelViewer::OnWindowResized(uint aWidth, uint aHeight)
{
  myCamera.myWidth = myWindow->GetWidth();
  myCamera.myHeight = myWindow->GetHeight();
  myCamera.UpdateProjection();
}

void Test_ModelViewer::OnUpdate(bool aDrawProperties)
{
  myCameraController.Update(0.016f, *myInput);
}

void Test_ModelViewer::OnRender()
{
  CommandList* ctx = RenderCore::BeginCommandList(CommandListType::Graphics);
    
  RenderGrid(ctx);
  RenderScene(ctx);

  RenderCore::ExecuteAndFreeCommandList(ctx);
}

void Test_ModelViewer::RenderGrid(Fancy::CommandList* ctx)
{
  ctx->SetViewport(glm::uvec4(0, 0, myWindow->GetWidth(), myWindow->GetHeight()));
  ctx->SetClipRect(glm::uvec4(0, 0, myWindow->GetWidth(), myWindow->GetHeight()));
  ctx->SetRenderTarget(myOutput->GetBackbufferRtv(), myOutput->GetDepthStencilDsv());

  ctx->SetDepthStencilState(nullptr);
  ctx->SetBlendState(nullptr);
  ctx->SetCullMode(CullMode::NONE);
  ctx->SetFillMode(FillMode::SOLID);
  ctx->SetWindingOrder(WindingOrder::CCW);

  ctx->SetShaderPipeline(myDebugGeoShader);

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
  ctx->BindConstantBuffer(&cbuffer_debugGeo, sizeof(cbuffer_debugGeo), "cbPerObject");

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

void Test_ModelViewer::RenderScene(Fancy::CommandList* ctx)
{
  ctx->SetViewport(glm::uvec4(0, 0, myWindow->GetWidth(), myWindow->GetHeight()));
  ctx->SetClipRect(glm::uvec4(0, 0, myWindow->GetWidth(), myWindow->GetHeight()));
  ctx->SetRenderTarget(myOutput->GetBackbufferRtv(), myOutput->GetDepthStencilDsv());

  ctx->SetDepthStencilState(nullptr);
  ctx->SetBlendState(nullptr);
  ctx->SetCullMode(CullMode::NONE);
  ctx->SetFillMode(FillMode::SOLID);
  ctx->SetWindingOrder(WindingOrder::CCW);

  ctx->SetTopologyType(TopologyType::TRIANGLE_LIST);
  ctx->SetShaderPipeline(myUnlitTexturedShader);
  ctx->BindSampler(mySampler.get(), "sampler_default");

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
    ctx->BindConstantBuffer(&cbuffer_perObject, sizeof(cbuffer_perObject), "cbPerObject");

    Material* mat = model->myMaterial.get();
    const GpuResourceView* diffuseTex = mat->mySemanticTextures[(uint)TextureSemantic::BASE_COLOR].get();
    if (diffuseTex)
      ctx->BindResourceView(diffuseTex, "tex_diffuse");

    Mesh* mesh = model->myMesh.get();
    for (SharedPtr<GeometryData>& geometry : mesh->myGeometryDatas)
    {
      const GeometryVertexLayout& layout = geometry->getGeometryVertexLayout();
      ctx->SetTopologyType(layout.myTopology);
      ctx->BindVertexBuffer(geometry->getVertexBuffer(), layout.myStride);
      ctx->BindIndexBuffer(geometry->getIndexBuffer(), geometry->getIndexBuffer()->GetProperties().myElementSizeBytes);

      ctx->Render(geometry->getNumIndices(), 1, 0, 0, 0);
    }
  }
}
