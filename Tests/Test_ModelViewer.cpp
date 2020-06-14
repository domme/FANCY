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
#include "fancy_core/Mesh.h"
#include "fancy_core/TextureSampler.h"
#include "fancy_core/GeometryData.h"
#include "fancy_core/ShaderPipeline.h"
#include "fancy_core/Shader.h"
#include "fancy_core/Texture.h"
#include "fancy_core/StringUtil.h"
#include "fancy_imgui/imgui.h"

using namespace Fancy;

bool ourDrawInstanced = true;

static SharedPtr<ShaderPipeline> locLoadShader(const char* aShaderPath, const char* aMainVtxFunction = "main", const char* aMainFragmentFunction = "main", const char* someDefines = nullptr)
{
  DynamicArray<String> defines;
  if (someDefines)
    StringUtil::Tokenize(someDefines, ",", defines);

  ShaderPipelineDesc pipelineDesc;

  ShaderDesc* shaderDesc = &pipelineDesc.myShader[(uint)ShaderStage::VERTEX];
  shaderDesc->myPath = aShaderPath;
  shaderDesc->myMainFunction = aMainVtxFunction;
  for (const String& str : defines)
    shaderDesc->myDefines.push_back(str);

  shaderDesc = &pipelineDesc.myShader[(uint)ShaderStage::FRAGMENT];
  shaderDesc->myPath = aShaderPath;
  shaderDesc->myMainFunction = aMainFragmentFunction;
  for (const String& str : defines)
    shaderDesc->myDefines.push_back(str);

  return RenderCore::CreateShaderPipeline(pipelineDesc);
}

//---------------------------------------------------------------------------//

Test_ModelViewer::Test_ModelViewer(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow,
  Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState)
  : Test(aRuntime, aWindow, aRenderOutput, anInputState, "Model Viewer")
  , myCameraController(&myCamera)
{
  myAssetManager.reset(new AssetManager());

  myUnlitTexturedShader = locLoadShader("Unlit_Textured.hlsl");
  ASSERT(myUnlitTexturedShader != nullptr);

  myInstancedUnlitTexturedShader = locLoadShader("Unlit_Textured.hlsl", "main", "main", "INSTANCED");
  ASSERT(myInstancedUnlitTexturedShader != nullptr);

  myUnlitVertexColorShader = locLoadShader("Unlit_Colored.hlsl");
  ASSERT(myUnlitVertexColorShader != nullptr);

  myDebugGeoShader = locLoadShader("DebugGeo_Colored.hlsl");
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

  StaticArray<VertexShaderAttributeDesc, 16> vertexAttributes;
  vertexAttributes.Add({ VertexAttributeSemantic::POSITION, 0u, DataFormat::RGB_32F });
  vertexAttributes.Add({ VertexAttributeSemantic::TEXCOORD, 0u, DataFormat::RG_32F });

  const bool importSuccess = ModelLoader::LoadFromFile("models/cube.obj", vertexAttributes, *(myAssetManager.get()), myScene);
  ASSERT(importSuccess);

  VertexInputLayoutProperties instancedVertexLayoutProps = myScene.myModels[0]->myMesh->myGeometryDatas[0]->GetVertexInputLayout()->myProperties;
  instancedVertexLayoutProps.myAttributes.Add({ DataFormat::RGB_32F, VertexAttributeSemantic::POSITION, 1u, 1u });
  instancedVertexLayoutProps.myBufferBindings.Add({ 12u, VertexInputRate::PER_INSTANCE });
  myInstancedVertexLayout = RenderCore::CreateVertexInputLayout(instancedVertexLayoutProps);

  int numInstancesOneSide = 20;
  int numInstances = numInstancesOneSide * numInstancesOneSide * numInstancesOneSide;
  myNumInstances = numInstances;
  float offsetBetweenInstances = 7.0f;
  DynamicArray<glm::float3> instancePositions;
  instancePositions.reserve(numInstances);
  for (int x = -numInstancesOneSide / 2; x < numInstancesOneSide / 2; ++x)
    for (int y = -numInstancesOneSide / 2; y < numInstancesOneSide / 2; ++y)
      for (int z = -numInstancesOneSide / 2; z < numInstancesOneSide / 2; ++z)
        instancePositions.push_back(glm::float3(x * offsetBetweenInstances, y * offsetBetweenInstances, z * offsetBetweenInstances));

  GpuBufferProperties bufferProps;
  bufferProps.myBindFlags = (uint) GpuBufferBindFlags::VERTEX_BUFFER;
  bufferProps.myElementSizeBytes = sizeof(glm::float3);
  bufferProps.myNumElements = numInstances;
  myInstancePositions = RenderCore::CreateBuffer(bufferProps, "Test_ModelViewer/InstancePositions", instancePositions.data());
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

  if (aDrawProperties)
  {
    ImGui::Checkbox("Instanced", &ourDrawInstanced);
  }
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

  ctx->SetShaderPipeline(myDebugGeoShader.get());

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
    glm::float4 myColor;
  };

  GridGeoVertex vertices[4] = {
    { { 0.0f, 0.0f, -1.0f }, {0,0, 1.0f,1.0f} },
    { { 0.0f, 0.0f, 1.0f }, {0.0f, 0.0f, 1.0f, 1.0f} },
    { { -1.0f, 0.0f, 0.0f }, {1.0f, 0.0f, 0.0f, 1.0f } },
    { { 1.0f, 0.0f, 0.0f },{1.0f, 0.0f, 0.0f, 1.0f } }
  };
  ctx->BindVertexBuffer(vertices, sizeof(vertices));

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
  ctx->SetShaderPipeline(ourDrawInstanced ? myInstancedUnlitTexturedShader.get() : myUnlitTexturedShader.get());
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
      const VertexInputLayout* layout = geometry->GetVertexInputLayout();
      ctx->SetTopologyType(TopologyType::TRIANGLE_LIST);
      ctx->SetVertexInputLayout(ourDrawInstanced ? myInstancedVertexLayout.get() : layout);

      uint64 offsets[] = { 0u, 0u };
      uint64 sizes[] = { geometry->GetVertexBuffer()->GetByteSize(), myInstancePositions->GetByteSize() };
      const GpuBuffer* buffers[] = { geometry->GetVertexBuffer(), myInstancePositions.get() };
      ctx->BindVertexBuffers(buffers, offsets, sizes, ourDrawInstanced ? 2u : 1u);
      ctx->BindIndexBuffer(geometry->GetIndexBuffer(), geometry->GetIndexBuffer()->GetProperties().myElementSizeBytes);

      ctx->Render(geometry->GetNumIndices(), (uint) myNumInstances, 0, 0, 0);
    }
  }
}
