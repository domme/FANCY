#include "RenderingProcessForward.h"
#include "RenderCore.h"
#include "Scene.h"
#include "Fancy.h"
#include "RenderQueues.h"
#include "LightComponent.h"
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
#include "CommandListType.h"
#include "CommandContext.h"
#include "Texture.h"

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
  RenderingProcessForward::RenderingProcessForward()
  {
    
  }
//---------------------------------------------------------------------------//
  RenderingProcessForward::~RenderingProcessForward()
  {
    myTestTexture.reset();
    myComputeProgram.reset();

    myDefaultObjectShaderState.reset();
    myBlendStateAdd.reset();

    myFsTextureShaderState.reset();
    myFullscreenQuad.reset();

    myPerDrawData.reset();
    myPerViewportData.reset();
    myPerLightData.reset();
    myPerCameraData.reset();
    myPerMaterialData.reset();
    myPerFrameData.reset();
  }
//---------------------------------------------------------------------------//
  void RenderingProcessForward::Startup()
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

    // Create Fullscreen-quad geometry
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

      GpuProgramPipelineDesc pipelineDesc;
      GpuProgramDesc* shaderDesc = &pipelineDesc.myGpuPrograms[(uint32)ShaderStage::VERTEX];
      shaderDesc->myShaderFileName = "FullscreenQuad";
      shaderDesc->myMainFunction = "main";
      shaderDesc->myShaderStage = (uint32)ShaderStage::VERTEX;
      shaderDesc = &pipelineDesc.myGpuPrograms[(uint32)ShaderStage::FRAGMENT];
      shaderDesc->myShaderFileName = "FullscreenQuad";
      shaderDesc->myShaderStage = (uint32)ShaderStage::FRAGMENT;
      shaderDesc->myMainFunction = "main_textured";
      myFsTextureShaderState = RenderCore::CreateGpuProgramPipeline(pipelineDesc);
      ASSERT(myFsTextureShaderState != nullptr, "Failed creating fullscreen texture shader state");
    }


    // Create default object shader state
    {
      GpuProgramPipelineDesc pipelineDesc;
      GpuProgramDesc* programDesc = &pipelineDesc.myGpuPrograms[(uint32) ShaderStage::VERTEX];
      programDesc->myShaderStage = static_cast<uint32>(ShaderStage::VERTEX);
      programDesc->myShaderFileName = "MaterialForward";

      programDesc = &pipelineDesc.myGpuPrograms[(uint32)ShaderStage::FRAGMENT];
      programDesc->myShaderStage = static_cast<uint32>(ShaderStage::FRAGMENT);
      programDesc->myShaderFileName = "MaterialForward";

      myDefaultObjectShaderState = RenderCore::CreateGpuProgramPipeline(pipelineDesc);
      ASSERT(myDefaultObjectShaderState != nullptr);
    }

    // Create render states
    {
      BlendStateDesc blendAddDesc;
      blendAddDesc.myBlendEnabled[0] = true;
      blendAddDesc.mySrcBlend[0] = static_cast<uint32>(BlendInput::ONE);
      blendAddDesc.myDestBlend[0] = static_cast<uint32>(BlendInput::ONE);
      blendAddDesc.myBlendOp[0] = static_cast<uint32>(BlendOp::ADD);
      myBlendStateAdd = RenderCore::CreateBlendState(blendAddDesc);
    }

    // Tests:
    _DebugLoadComputeShader();
  }
//---------------------------------------------------------------------------//
  void RenderingProcessForward::UpdatePerFrameData(const Time& aClock) const
  {
    PerFrameData frameData;
    frameData.c_TimeParamters = glm::float4(aClock.GetDelta(), aClock.GetElapsed(), 0.0f, 0.0f);
    RenderCore::UpdateBufferData(myPerFrameData.get(), &frameData, sizeof(frameData));
  }
//---------------------------------------------------------------------------//
  void RenderingProcessForward::UpdatePerCameraData(const Scene::CameraComponent* aCamera) const
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
  void RenderingProcessForward::UpdatePerLightData(const Scene::LightComponent* aLight, const Scene::CameraComponent* aCamera) const
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
  void RenderingProcessForward::UpdatePerDrawData(const Scene::CameraComponent* aCamera, const glm::float4x4& aWorldMat) const
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
  void RenderingProcessForward::_DebugLoadComputeShader()
  {
    // Compute shader
    GpuProgramPipelineDesc desc;
    desc.myGpuPrograms[(uint)ShaderStage::COMPUTE].myShaderStage = (uint)ShaderStage::COMPUTE;
    desc.myGpuPrograms[(uint)ShaderStage::COMPUTE].myShaderFileName = "ComputeMipmapCS";
    
    myComputeProgram = RenderCore::CreateGpuProgramPipeline(desc);

    // Texture
    TextureParams texParams;
    texParams.myIsExternalTexture = false;
    texParams.eFormat = DataFormat::RGBA_8;
    texParams.myIsRenderTarget = false;
    texParams.myIsShaderWritable = true;
    texParams.u16Width = 32;
    texParams.u16Height = 32;

    DataFormatInfo formatInfo(texParams.eFormat);

    std::vector<uint8> data;
    data.resize(texParams.u16Width * texParams.u16Height * formatInfo.mySizeBytes, 0);

    TextureUploadData texData;
    texData.myTotalSizeBytes = texParams.u16Width * texParams.u16Height * formatInfo.mySizeBytes;
    texData.mySliceSizeBytes = texData.myTotalSizeBytes;
    texData.myRowSizeBytes = texParams.u16Width * formatInfo.mySizeBytes;
    texData.myPixelSizeBytes = formatInfo.mySizeBytes;
    texData.myData = &data[0];
    
    myTestTexture = RenderCore::CreateTexture(texParams, &texData, 1u);
    ASSERT(myTestTexture != nullptr);
  }
//---------------------------------------------------------------------------//
  void RenderingProcessForward::_DebugExecuteComputeShader()
  {
    
  }
//---------------------------------------------------------------------------//
  void RenderingProcessForward::Tick(const GraphicsWorld* aWorld, const RenderOutput* anOutput, const Time& aClock)
  {
    _DebugExecuteComputeShader();

    PopulateRenderQueues(aWorld);
    FlushRenderQueues(aWorld, anOutput, aClock);
  }
//---------------------------------------------------------------------------//
  void RenderingProcessForward::PopulateRenderQueues(const GraphicsWorld* aWorld)
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
  void RenderingProcessForward::FlushRenderQueues(const GraphicsWorld* aWorld, const RenderOutput* anOutput, const Time& aClock) const
  {
    if (myRenderQueueFromCamera.IsEmpty())
      return;

    const Scene::Scene* scene = aWorld->GetScene();
    const Scene::CameraComponent* camera = scene->getActiveCamera();
    const RenderWindow* renderWindow = anOutput->GetWindow();

    UpdatePerFrameData(aClock);
    UpdatePerCameraData(camera);

    CommandContext* context = RenderCore::AllocateContext(CommandListType::Graphics);

    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    context->ClearRenderTarget(anOutput->GetBackbuffer(), clearColor);

    const float clearDepth = 1.0f;
    uint8 clearStencil = 0u;
    context->ClearDepthStencilTarget(anOutput->GetDefaultDepthStencilBuffer(), clearDepth, clearStencil);

    context->SetViewport(glm::uvec4(0, 0, renderWindow->GetWidth(), renderWindow->GetHeight()));
    context->SetClipRect(glm::uvec4(0, 0, renderWindow->GetWidth(), renderWindow->GetHeight()));
    context->SetRenderTarget(anOutput->GetBackbuffer(), 0u);
    context->SetDepthStencilRenderTarget(anOutput->GetDefaultDepthStencilBuffer());

    context->SetDepthStencilState(nullptr);
    // context->SetBlendState(myBlendStateAdd);
    context->SetBlendState(nullptr);
    context->SetCullMode(CullMode::NONE);
    context->SetFillMode(FillMode::SOLID);
    context->SetWindingOrder(WindingOrder::CCW);

    context->SetGpuProgramPipeline(myDefaultObjectShaderState);
    context->BindResource(myPerLightData.get(), DescriptorType::CONSTANT_BUFFER, 0);
    context->BindResource(myPerDrawData.get(), DescriptorType::CONSTANT_BUFFER, 1);

    const Scene::LightList& aLightList = scene->getCachedLights();
    for (uint32 iLight = 0u; iLight < aLightList.size(); ++iLight)
    {
      const Scene::LightComponent* lightComp = aLightList[iLight];
      UpdatePerLightData(lightComp, camera);

      const auto& renderQueueItems = myRenderQueueFromCamera.GetItems();
      for (uint32 iItem = 0u, num = renderQueueItems.size(); iItem < num; ++iItem)
      {
        const RenderQueueItem& item = renderQueueItems[iItem];

        UpdatePerDrawData(camera, item.myWorldMat);

        BindResources_ForwardColorPass(context, item.myMaterial);
        
        context->RenderGeometry(item.myGeometry);
      }
    }

    context->ExecuteAndReset(true);
    RenderCore::FreeContext(context);
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
  void RenderingProcessForward::BindResources_ForwardColorPass(CommandContext* aContext, const Material* aMaterial)
  {
    const uint32 kNumTextures = 3u;
    const Descriptor* descriptorsToBind[kNumTextures];

    descriptorsToBind[0] = RenderCore::GetDefaultDiffuseTexture()->GetDescriptor(DescriptorType::DEFAULT_READ);
    descriptorsToBind[1] = RenderCore::GetDefaultNormalTexture()->GetDescriptor(DescriptorType::DEFAULT_READ);
    descriptorsToBind[2] = RenderCore::GetDefaultMaterialTexture()->GetDescriptor(DescriptorType::DEFAULT_READ);

    for (const MaterialTexture& matTexture : aMaterial->myTextures)
    {
      const uint32 regIndex = locTexSemanticToRegIndex_MaterialDefault(matTexture.mySemantic);
      if (regIndex != ~0)
      {
        ASSERT(regIndex < kNumTextures);
        descriptorsToBind[regIndex] = matTexture.myTexture->GetDescriptor(DescriptorType::DEFAULT_READ);
      }
    }

    aContext->BindDescriptorSet(descriptorsToBind, kNumTextures, 2);
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering