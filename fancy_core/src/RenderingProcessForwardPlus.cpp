#include "RenderingProcessForwardPlus.h"
#include "RenderCore.h"
#include "Scene.h"
#include "Fancy.h"
#include "RenderQueues.h"
#include "LightComponent.h"
#include "RenderContext.h"
#include "GpuProgramPipeline.h"
#include "GeometryData.h"
#include "TimeManager.h"
#include "CameraComponent.h"
#include "SceneNode.h"
#include "RenderWindow.h"
#include "GraphicsWorld.h"
#include "Model.h"
#include "Mesh.h"
#include "BlendState.h"
#include "RenderOutput.h"
#include "Texture.h"
#include "DepthStencilState.h"

namespace Fancy { namespace Rendering {

  namespace {
  //---------------------------------------------------------------------------//
    struct PerFrameData
    {
      glm::float4 c_TimeParamters;
    };
  //---------------------------------------------------------------------------//
    struct PerViewportData
    {
      glm::float4 c_RenderTargetSize;
    };
  //---------------------------------------------------------------------------//
    struct PerCameraData
    {
      glm::float4x4 c_ViewMatrix;
      glm::float4x4 c_ViewInverseMatrix;
      glm::float4x4 c_ProjectionMatrix;
      glm::float4x4 c_ProjectionInverseMatrix;
      glm::float4x4 c_ViewProjectionMatrix;
      glm::float4x4 c_ViewProjectionInverseMatrix;
      glm::float4 c_NearFarParameters;
      glm::float4 c_CameraPosWS;
    };
  //---------------------------------------------------------------------------//
    struct PerLightData
    {
      // xyz: ColorIntensity
      // w: LightType (Dir, Point, Spot, Area)
      glm::float4 c_LightParameters;
      glm::float4 c_PointSpotParameters;
      glm::float3 c_LightPosWS;
      glm::float3 c_LightPosVS;
      glm::float3 c_LightDirWS;
      glm::float3 c_LightDirVS;
    };
  //---------------------------------------------------------------------------//  
    struct PerMaterialData
    {
      glm::float4 c_MatDiffIntensity;
      glm::float4 c_MatSpecIntensity;
    };
  //---------------------------------------------------------------------------//  
    struct PerDrawData
    {
      glm::float4x4 c_WorldMatrix;
      glm::float4x4 c_WorldInverseMatrix;
      glm::float4x4 c_WorldViewMatrix;
      glm::float4x4 c_WorldViewInverseMatrix;
      glm::float4x4 c_WorldViewProjectionMatrix;
      glm::float4x4 c_WorldViewProjectionInverseMatrix;
    };
  //---------------------------------------------------------------------------//  
  }
//---------------------------------------------------------------------------//
  RenderingProcessForwardPlus::RenderingProcessForwardPlus()
  {
    
  }
//---------------------------------------------------------------------------//
  RenderingProcessForwardPlus::~RenderingProcessForwardPlus()
  {
    myBlendStateAdd.reset();
    myFullscreenQuad.reset();

    myPerDrawData.reset();
    myPerViewportData.reset();
    myPerLightData.reset();
    myPerCameraData.reset();
    myPerMaterialData.reset();
    myPerFrameData.reset();
  }
//---------------------------------------------------------------------------//
  void RenderingProcessForwardPlus::Startup()
  {
    // Create constant buffers
    GpuBufferCreationParams bufferParams;
    bufferParams.myUsageFlags = static_cast<uint32>(GpuBufferUsage::CONSTANT_BUFFER);
    bufferParams.uAccessFlags = (uint32)GpuResourceAccessFlags::WRITE
                              | (uint32)GpuResourceAccessFlags::COHERENT
                              | (uint32)GpuResourceAccessFlags::DYNAMIC
                              | (uint32)GpuResourceAccessFlags::PERSISTENT_LOCKABLE;
    bufferParams.uElementSizeBytes = sizeof(float);

    // Per frame
    {
      PerFrameData initialData;
      memset(&initialData, 0, sizeof(initialData));
      bufferParams.uNumElements = sizeof(initialData) / sizeof(float);
      myPerFrameData = RenderCore::CreateBuffer(bufferParams, &initialData);
      ASSERT(myPerFrameData != nullptr);
    }

    // Per Material
    {
      PerMaterialData initialData;
      memset(&initialData, 0, sizeof(initialData));
      bufferParams.uNumElements = sizeof(initialData) / sizeof(float);
      myPerMaterialData = RenderCore::CreateBuffer(bufferParams, &initialData);
      ASSERT(myPerMaterialData != nullptr);
    }

    // Per Camera
    {
      PerCameraData initialData;
      memset(&initialData, 0, sizeof(initialData));
      bufferParams.uNumElements = sizeof(initialData) / sizeof(float);
      myPerCameraData = RenderCore::CreateBuffer(bufferParams, &initialData);
      ASSERT(myPerCameraData != nullptr);
    }

    // Per Light
    {
      PerLightData initialData;
      memset(&initialData, 0, sizeof(initialData));
      bufferParams.uNumElements = sizeof(initialData) / sizeof(float);
      myPerLightData = RenderCore::CreateBuffer(bufferParams, &initialData);
      ASSERT(myPerLightData != nullptr);
    }

    // Per View
    {
      PerViewportData initialData;
      memset(&initialData, 0, sizeof(initialData));
      bufferParams.uNumElements = sizeof(initialData) / sizeof(float);
      myPerViewportData = RenderCore::CreateBuffer(bufferParams, &initialData);
      ASSERT(myPerViewportData != nullptr);
    }

    // Per Draw
    {
      PerDrawData initialData;
      memset(&initialData, 0, sizeof(initialData));
      bufferParams.uNumElements = sizeof(initialData) / sizeof(float);
      myPerDrawData = RenderCore::CreateBuffer(bufferParams, &initialData);
      ASSERT(myPerDrawData != nullptr);
    }

    // Create Fullscreen-quad geometry and debug shaders
    {
      struct Vertex
      {
        glm::vec3 pos;
      };

      Vertex quadVertices[4];
      memset(quadVertices, 0, sizeof(quadVertices));

      // 0---1
      // | / |
      // 3---2

      float depth = 1.0f;
      quadVertices[0].pos = glm::vec3(-1.0f, 1.0f, depth);
      quadVertices[1].pos = glm::vec3(1.0f, 1.0f, depth);
      quadVertices[2].pos = glm::vec3(1.0f, -1.0f, depth);
      quadVertices[3].pos = glm::vec3(-1.0f, -1.0f, depth);

      uint16 quadIndices[6] =
      {
        0, 3, 1,
        1, 3, 2
      };

      GpuBufferCreationParams bufferParams;
      bufferParams.uNumElements = 4u;
      bufferParams.uElementSizeBytes = sizeof(Vertex);
      bufferParams.myUsageFlags = (uint32)GpuBufferUsage::VERTEX_BUFFER;
      bufferParams.uAccessFlags = (uint32)GpuResourceAccessFlags::NONE;
      SharedPtr<GpuBuffer> vertexBuffer = RenderCore::CreateBuffer(bufferParams, quadVertices);

      bufferParams.uNumElements = 6u;
      bufferParams.uElementSizeBytes = sizeof(uint16);
      bufferParams.myUsageFlags = (uint32)GpuBufferUsage::INDEX_BUFFER;
      bufferParams.uAccessFlags = (uint32)GpuResourceAccessFlags::NONE;
      SharedPtr<GpuBuffer> indexBuffer = RenderCore::CreateBuffer(bufferParams, quadIndices);

      myFullscreenQuad = std::make_shared<Geometry::GeometryData>();
      myFullscreenQuad->setVertexBuffer(vertexBuffer);
      myFullscreenQuad->setIndexBuffer(indexBuffer);
    }

    // Depth prepass shader state
    {
      GpuProgramPipelineDesc pipelineDesc;
      GpuProgramDesc* programDesc = &pipelineDesc.myGpuPrograms[(uint32)ShaderStage::VERTEX];
      programDesc->myShaderStage = static_cast<uint32>(ShaderStage::VERTEX);
      programDesc->myShaderFileName = "Forwardplus/DepthPrepass";

      programDesc = &pipelineDesc.myGpuPrograms[(uint32)ShaderStage::FRAGMENT];
      programDesc->myShaderStage = static_cast<uint32>(ShaderStage::FRAGMENT);
      programDesc->myShaderFileName = "ForwardPlus/DepthPrepass";

      myDepthPrepassObjectShader = RenderCore::CreateGpuProgramPipeline(pipelineDesc);
      ASSERT(myDepthPrepassObjectShader != nullptr);
    }

    // Additive blend state
    {
      BlendStateDesc blendStateDesc;
      blendStateDesc.myBlendEnabled[0] = true;
      blendStateDesc.mySrcBlend[0] = static_cast<uint32>(BlendInput::ONE);
      blendStateDesc.myDestBlend[0] = static_cast<uint32>(BlendInput::ONE);
      blendStateDesc.myBlendOp[0] = static_cast<uint32>(BlendOp::ADD);
      myBlendStateAdd = RenderCore::CreateBlendState(blendStateDesc);
      ASSERT(myBlendStateAdd != nullptr);
    }

    // No-color write blend state
    {
      BlendStateDesc blendStateDesc;
      blendStateDesc.myBlendEnabled[0] = false;
      blendStateDesc.myRTwriteMask[0] = 0;
      myBlendStateNoColors = RenderCore::CreateBlendState(blendStateDesc);
      ASSERT(myBlendStateNoColors != nullptr);
    }

    // Pass-through depth stencil state
    {
      DepthStencilStateDesc desc;
      desc.myDepthTestEnabled = false;
      desc.myStencilEnabled = false;
      myDepthStencil_NoDepthTest = RenderCore::CreateDepthStencilState(desc);
      ASSERT(myDepthStencil_NoDepthTest != nullptr);
    }

    StartupDebug();
  }
//---------------------------------------------------------------------------//
  void RenderingProcessForwardPlus::StartupDebug()
  {
    GpuBufferCreationParams bufferParams;
    bufferParams.myUsageFlags = static_cast<uint32>(GpuBufferUsage::CONSTANT_BUFFER);
    bufferParams.uAccessFlags = (uint32)GpuResourceAccessFlags::WRITE
                                | (uint32)GpuResourceAccessFlags::COHERENT
                                | (uint32)GpuResourceAccessFlags::DYNAMIC
                                | (uint32)GpuResourceAccessFlags::PERSISTENT_LOCKABLE;
    bufferParams.uElementSizeBytes = sizeof(float);

    // Debug texture params
    {
      struct DebugTexParams
      {
        glm::vec4 myParams;
        glm::vec4 _padding;
      };
      DebugTexParams params = {};

      bufferParams.uNumElements = 1;
      bufferParams.uElementSizeBytes = sizeof(params);
      myDebugTextureParams = RenderCore::CreateBuffer(bufferParams, &params);
      ASSERT(myDebugTextureParams != nullptr);
    }

    // Depth buffer copy texture
    {
      TextureParams texParams;
      texParams.myIsExternalTexture = false;
      texParams.eFormat = DataFormat::RGBA_8;
      texParams.myIsRenderTarget = true;
      texParams.myIsShaderWritable = false;
      texParams.u16Width = 1280u;
      texParams.u16Height = 720u;
      myDepthBufferDebugTex = RenderCore::CreateTexture(texParams);
    }

    GpuProgramPipelineDesc pipelineDesc;
    GpuProgramDesc* vertexShaderDesc = &pipelineDesc.myGpuPrograms[(uint32)ShaderStage::VERTEX];
    GpuProgramDesc* fragmentShaderDesc = &pipelineDesc.myGpuPrograms[(uint32)ShaderStage::FRAGMENT];
    vertexShaderDesc->myShaderFileName = "Forwardplus/DebugTexture";
    vertexShaderDesc->myMainFunction = "main";
    vertexShaderDesc->myShaderStage = (uint32)ShaderStage::VERTEX;

    fragmentShaderDesc->myShaderFileName = "Forwardplus/DebugTexture";
    fragmentShaderDesc->myShaderStage = (uint32)ShaderStage::FRAGMENT;
    fragmentShaderDesc->myMainFunction = "main";
    myDefaultTextureDebugShader = RenderCore::CreateGpuProgramPipeline(pipelineDesc);
    ASSERT(myDefaultTextureDebugShader != nullptr, "Failed creating fullscreen texture shader state");

    fragmentShaderDesc->myMainFunction = "main_depthBuffer";
    myDepthBufferDebugShader = RenderCore::CreateGpuProgramPipeline(pipelineDesc);
    ASSERT(myDepthBufferDebugShader != nullptr, "Failed creating depth buffer debug shader state");
  }
//---------------------------------------------------------------------------//
  void RenderingProcessForwardPlus::UpdatePerFrameData(const Time& aClock) const
  {
    PerFrameData frameData;
    frameData.c_TimeParamters = glm::float4(aClock.GetDelta(), aClock.GetElapsed(), 0.0f, 0.0f);
    RenderCore::UpdateBufferData(myPerFrameData.get(), &frameData, sizeof(frameData));
  }
//---------------------------------------------------------------------------//
  void RenderingProcessForwardPlus::UpdatePerCameraData(const Scene::CameraComponent* aCamera) const
  {
    const glm::mat4& viewMat = aCamera->getView();
    const glm::mat4& viewInvMat = aCamera->getViewInv();
    const glm::mat4& projMat = aCamera->getProjection();
    const glm::mat4& viewProjMat = aCamera->getViewProjection();
    glm::mat4 projInvMat(glm::inverse(projMat));
    glm::mat4 viewProjInv(glm::inverse(viewProjMat));
    const float fNear = aCamera->getNearPlane();
    const float fFar = aCamera->getFarPlane();

    PerCameraData cBuffer;
    cBuffer.c_ViewMatrix = viewMat;
    cBuffer.c_ViewInverseMatrix = viewInvMat;
    cBuffer.c_ProjectionMatrix = projMat;
    cBuffer.c_ProjectionInverseMatrix = projInvMat;
    cBuffer.c_ViewProjectionMatrix = viewProjMat;
    cBuffer.c_ViewProjectionInverseMatrix = viewProjInv;
    cBuffer.c_NearFarParameters[0] = fNear;
    cBuffer.c_NearFarParameters[1] = fFar;
    cBuffer.c_NearFarParameters[2] = fNear / fFar;
    cBuffer.c_NearFarParameters[3] = 1.0f / fFar;
    cBuffer.c_CameraPosWS = glm::column(viewInvMat, 3);

    RenderCore::UpdateBufferData(myPerCameraData.get(), &cBuffer, sizeof(cBuffer));
  }
//---------------------------------------------------------------------------//
  void RenderingProcessForwardPlus::UpdatePerLightData(const Scene::LightComponent* aLight, const Scene::CameraComponent* aCamera) const
  {
    const Scene::Transform& lightTransform = aLight->getSceneNode()->getTransform();

    glm::vec4 lightParams(aLight->getColorIntensity(), static_cast<float>(aLight->getType()));
    glm::vec4 pointSpotParams(aLight->getFalloffStart(), aLight->getFalloffEnd(), aLight->getConeAngle(), 0.0f);
    glm::vec3 lightPosVS = static_cast<glm::vec3>(aCamera->getView() * glm::vec4(lightTransform.getPosition(), 1.0f));
    glm::vec3 lightDirVS = glm::normalize(static_cast<glm::mat3>(aCamera->getView()) * lightTransform.forward());

    PerLightData cBuffer;
    cBuffer.c_LightParameters = lightParams;
    cBuffer.c_PointSpotParameters = pointSpotParams;
    cBuffer.c_LightPosWS = lightTransform.getPosition();
    cBuffer.c_LightPosVS = lightPosVS;
    cBuffer.c_LightDirWS = lightTransform.forward();
    cBuffer.c_LightDirVS = lightDirVS;
    
    RenderCore::UpdateBufferData(myPerLightData.get(), &cBuffer, sizeof(cBuffer));
  }
//---------------------------------------------------------------------------//
  void RenderingProcessForwardPlus::UpdatePerDrawData(const Scene::CameraComponent* aCamera, const glm::float4x4& aWorldMat) const
  {
    const glm::mat4& viewMat = aCamera->getView();
    const glm::mat4& viewProjMat = aCamera->getViewProjection();
    const glm::mat4& worldMat = aWorldMat;
    const glm::mat4 worldInvMat(glm::affineInverse(worldMat));
    const glm::mat4 worldView(viewMat * worldMat);
    const glm::mat4 worldViewInv(glm::affineInverse(worldView));
    const glm::mat4 worldViewProj(viewProjMat * worldMat);
    const glm::mat4 worldViewProjInv(glm::inverse(worldViewProj));

    PerDrawData cBuffer;
    cBuffer.c_WorldMatrix = worldMat;
    cBuffer.c_WorldInverseMatrix = worldInvMat;
    cBuffer.c_WorldViewMatrix = worldView;
    cBuffer.c_WorldViewInverseMatrix = worldViewInv;
    cBuffer.c_WorldViewProjectionMatrix = worldViewProj;
    cBuffer.c_WorldViewProjectionInverseMatrix = worldViewProjInv;
  
    RenderCore::UpdateBufferData(myPerDrawData.get(), &cBuffer, sizeof(cBuffer));
  }
//---------------------------------------------------------------------------//
  void RenderingProcessForwardPlus::UpdateDebug(const RenderOutput* anOutput)
  {
    const uint depthBufferWidth = anOutput->GetDefaultDepthStencilBuffer()->GetParameters().u16Width;
    const uint depthBufferHeight = anOutput->GetDefaultDepthStencilBuffer()->GetParameters().u16Height;
    const TextureParams& currParams = myDepthBufferDebugTex->GetParameters();
    if (currParams.u16Width != depthBufferWidth ||
        currParams.u16Height != depthBufferHeight)
    {
      TextureParams params = currParams;
      params.u16Width = depthBufferWidth;
      params.u16Height = depthBufferHeight;
      myDepthBufferDebugTex->Create(params);
    }
  }
//---------------------------------------------------------------------------//
  void RenderingProcessForwardPlus::RenderDebug(const RenderOutput* anOutput)
  {
    const RenderWindow* renderWindow = anOutput->GetWindow();
    RenderContext* context = static_cast<RenderContext*>(RenderCore::AllocateContext(CommandListType::Graphics));

    const TextureParams& texParams = myDepthBufferDebugTex->GetParameters();

    struct Constants
    {
      glm::vec4 myParams;
      glm::vec4 _padding;
    };
    Constants constants = {};
    constants.myParams = glm::vec4(0.0f, 100.0f, 0.0f, 0.0f);
    RenderCore::UpdateBufferData(myDebugTextureParams.get(), &constants, sizeof(constants));

    context->RemoveAllRenderTargets();
    context->SetRenderTarget(myDepthBufferDebugTex.get(), 0u);
    context->SetViewport(glm::uvec4(0, 0, texParams.u16Width, texParams.u16Height));
    context->SetClipRect(glm::uvec4(0, 0, texParams.u16Width, texParams.u16Height));

    context->SetBlendState(myBlendStateNoColors);
    context->SetCullMode(CullMode::NONE);
    context->SetFillMode(FillMode::SOLID);
    context->SetWindingOrder(WindingOrder::CCW);

    context->SetGpuProgramPipeline(myDepthBufferDebugShader);
    context->BindResource(myDebugTextureParams.get(), DescriptorType::CONSTANT_BUFFER, 0);

    const Descriptor* textures[] = { anOutput->GetDefaultDepthStencilBuffer()->GetDescriptor(DescriptorType::DEFAULT_READ_DEPTH) };
    context->BindDescriptorSet(textures, 1, 1);

    context->RenderGeometry(myFullscreenQuad.get());

    context->ExecuteAndReset(true);
    RenderCore::FreeContext(context);
  }
//---------------------------------------------------------------------------//
  void RenderingProcessForwardPlus::Tick(const GraphicsWorld* aWorld, const RenderOutput* anOutput, const Time& aClock)
  {
    UpdateDebug(anOutput);

    PopulateRenderQueues(aWorld);

    UpdatePerFrameData(aClock);

    const Scene::CameraComponent* camera = aWorld->GetScene()->getActiveCamera();
    UpdatePerCameraData(camera);

    DepthPrepass(aWorld, anOutput, aClock);
    BuildLightTiles(aWorld, anOutput, aClock);

    RenderDebug(anOutput);
  }
//---------------------------------------------------------------------------//
  void RenderingProcessForwardPlus::PopulateRenderQueues(const GraphicsWorld* aWorld)
  {
    const Scene::Scene* scene = aWorld->GetScene();
    const Scene::CameraComponent* camera = scene->getActiveCamera();
    
    myRenderQueueFromCamera.Clear();
    const Scene::ModelList& modelComponents = scene->getCachedModels();

    for (uint i = 0u; i < modelComponents.size(); ++i)
    {
      const Scene::ModelComponent* modelComp = modelComponents[i];
      const Geometry::Model* model = modelComp->getModel();
      if (model == nullptr)
        continue;

      const std::vector<SharedPtr<Geometry::SubModel>>& subModels = model->getSubModelList();
      for (const SharedPtr<Geometry::SubModel>& subModel : subModels)
      {
        const Material* material = subModel->getMaterial();
        const Geometry::Mesh* mesh = subModel->getMesh();

        const Geometry::GeometryDataList& geometries = mesh->getGeometryDataList();
        for (uint iGeo = 0u; iGeo < geometries.size(); ++iGeo)
        {
          const Geometry::GeometryData* geo = geometries[iGeo];

          RenderQueueItem* item = myRenderQueueFromCamera.AddItem();
          item->myMaterial = material;
          item->myGeometry = geo;
          item->myWorldMat = modelComp->getSceneNode()->getTransform().getCachedWorld();
        }
      }
    }

    // TODO: Sort based on material
    // TODO: Optimize with instanced calls if materials and geometries are the same
  }
//---------------------------------------------------------------------------//
  void RenderingProcessForwardPlus::DepthPrepass(const GraphicsWorld* aWorld, const RenderOutput* anOutput, const Time& aClock)
  {
    if (myRenderQueueFromCamera.IsEmpty())
      return;

    const Scene::Scene* scene = aWorld->GetScene();
    const Scene::CameraComponent* camera = scene->getActiveCamera();
    const RenderWindow* renderWindow = anOutput->GetWindow();

    RenderContext* context = static_cast<RenderContext*>(RenderCore::AllocateContext(CommandListType::Graphics));

    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    context->ClearRenderTarget(anOutput->GetBackbuffer(), clearColor);

    const float clearDepth = 1.0f;
    uint8 clearStencil = 0u;
    context->ClearDepthStencilTarget(anOutput->GetDefaultDepthStencilBuffer(), clearDepth, clearStencil);

    context->SetViewport(glm::uvec4(0, 0, renderWindow->GetWidth(), renderWindow->GetHeight()));
    context->SetClipRect(glm::uvec4(0, 0, renderWindow->GetWidth(), renderWindow->GetHeight()));
    context->SetRenderTarget(anOutput->GetBackbuffer(), 0u);
    context->SetDepthStencilRenderTarget(anOutput->GetDefaultDepthStencilBuffer());

    context->SetBlendState(myBlendStateNoColors);
    context->SetCullMode(CullMode::NONE);
    context->SetFillMode(FillMode::SOLID);
    context->SetWindingOrder(WindingOrder::CCW);

    context->SetGpuProgramPipeline(myDepthPrepassObjectShader);
    context->BindResource(myPerDrawData.get(), DescriptorType::CONSTANT_BUFFER, 0);
        
    const auto& renderQueueItems = myRenderQueueFromCamera.GetItems();
    for (uint32 iItem = 0u, num = renderQueueItems.size(); iItem < num; ++iItem)
    {
      const RenderQueueItem& item = renderQueueItems[iItem];
      UpdatePerDrawData(camera, item.myWorldMat);
      context->RenderGeometry(item.myGeometry);
    }
    
    context->ExecuteAndReset(true);
    RenderCore::FreeContext(context);
  }
//---------------------------------------------------------------------------//
  void RenderingProcessForwardPlus::BuildLightTiles(const GraphicsWorld* aWorld, const RenderOutput* anOutput, const Time& aClock)
  {
  }
//---------------------------------------------------------------------------//
  static uint32 locTexSemanticToRegIndex_MaterialDefault(EMaterialTextureSemantic aSemantic)
  {
    switch (aSemantic)
    {
      case EMaterialTextureSemantic::BASE_COLOR: return 0;
      case EMaterialTextureSemantic::NORMAL: return 1;
      case EMaterialTextureSemantic::MATERIAL: return 2;
      default: return ~0;
    }
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering