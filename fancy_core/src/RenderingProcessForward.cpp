#include "RenderingProcessForward.h"
#include "Renderer.h"
#include "Scene.h"
#include "Fancy.h"
#include "SceneRenderDescription.h"
#include "MaterialPass.h"
#include "LightComponent.h"
#include "MaterialPassInstance.h"
#include "RenderContext.h"
#include "GpuProgramCompiler.h"
#include "GpuProgramPipeline.h"
#include "GeometryData.h"
#include "ResourceBinding.h"
#include "TimeManager.h"
#include "CameraComponent.h"
#include "SceneNode.h"

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

  //---------------------------------------------------------------------------//
    GpuProgramPipeline ourDummyPipeline;
    Geometry::GeometryData ourFsQuadGeometry;

    void locCreateFullscreenQuad()
    {
      GpuProgramDesc shaderDesc;
      shaderDesc.myShaderPath = "shader/DX12/FullscreenQuad.hlsl";
      shaderDesc.myShaderStage = (uint32)ShaderStage::VERTEX;
      GpuProgram* vertexShader = GpuProgramCompiler::createOrRetrieve(shaderDesc);

      shaderDesc.myShaderStage = (uint32)ShaderStage::FRAGMENT;
      GpuProgram* fragmentShader = GpuProgramCompiler::createOrRetrieve(shaderDesc);

      ourDummyPipeline.myGpuPrograms[(uint32)ShaderStage::VERTEX] = vertexShader;
      ourDummyPipeline.myGpuPrograms[(uint32)ShaderStage::FRAGMENT] = fragmentShader;
      ourDummyPipeline.myResourceInterface = vertexShader->GetResourceInterface();

      struct Vertex
      {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec3 bitangent;
        glm::vec2 uv;
      };

      Vertex quadVertices[4];
      memset(quadVertices, 0, sizeof(quadVertices));

      // 0---1
      // | / |
      // 3---2

      quadVertices[0].pos = glm::vec3(-0.5f, -0.5f, 0.5f);
      quadVertices[1].pos = glm::vec3(0.5f, -0.5f, 0.5f);
      quadVertices[2].pos = glm::vec3(0.5f, 0.5f, 0.5f);
      quadVertices[3].pos = glm::vec3(-0.5f, 0.5f, 0.5f);

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

      GpuBuffer* vertexBuffer = new GpuBuffer();
      vertexBuffer->create(bufferParams, quadVertices);

      bufferParams.uNumElements = 6u;
      bufferParams.uElementSizeBytes = sizeof(uint16);
      bufferParams.myUsageFlags = (uint32)GpuBufferUsage::INDEX_BUFFER;
      bufferParams.uAccessFlags = (uint32)GpuResourceAccessFlags::NONE;

      GpuBuffer* indexBuffer = new GpuBuffer();
      indexBuffer->create(bufferParams, quadIndices);

      ourFsQuadGeometry.setVertexBuffer(vertexBuffer);
      ourFsQuadGeometry.setIndexBuffer(indexBuffer);
    }
  //---------------------------------------------------------------------------//
  }
//---------------------------------------------------------------------------//
  RenderingProcessForward::RenderingProcessForward()
  {
    
  }
//---------------------------------------------------------------------------//
  RenderingProcessForward::~RenderingProcessForward()
  {

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
  }
//---------------------------------------------------------------------------//
  void RenderingProcessForward::UpdatePerFrameData() const
  {
    PerFrameData frameData;
    frameData.c_TimeParamters = glm::float4(Time::getDeltaTime(), Time::getElapsedTime(), 0.0f, 0.0f);

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
  void RenderingProcessForward::Tick(float _dt)
  {
    Scene::Scene* pScene = Fancy::GetCurrentScene().get();
    RenderOutput& renderer = *Fancy::GetCurrentRenderOutput();
    
    Scene::SceneRenderDescription renderDesc;
    pScene->gatherRenderItems(&renderDesc);

    UpdatePerFrameData();

    const Scene::CameraComponent* camera = pScene->getActiveCamera();

    UpdatePerCameraData(camera);

    RenderContext* context = RenderContext::AllocateContext();
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    context->ClearRenderTarget(renderer.GetBackbuffer(), clearColor);

    const float clearDepth = 1.0f;
    uint8 clearStencil = 0u;
    context->ClearDepthStencilTarget(renderer.GetDefaultDepthStencilBuffer(), clearDepth, clearStencil);

    context->setViewport(glm::uvec4(0, 0, 1280, 720));
    context->setRenderTarget(renderer.GetBackbuffer(), 0u);
    context->setDepthStencilRenderTarget(renderer.GetDefaultDepthStencilBuffer());

    const Scene::RenderingItemList& forwardRenderList = renderDesc.techniqueItemList[(uint32) Rendering::EMaterialPass::SOLID_FORWARD];
    // TODO: Sort based on material-pass

    const Scene::LightList& aLightList = pScene->getCachedLights();

    context->setRenderTarget(renderer.GetBackbuffer(), 0u);

    const MaterialPass* pCachedMaterialPass = nullptr;
    for (uint32 iLight = 0u; iLight < aLightList.size(); ++iLight)
    {
      const Scene::LightComponent* aLight = aLightList[iLight];
      UpdatePerLightData(aLight, pScene->getActiveCamera());

      for (uint32 iRenderItem = 0u; iRenderItem < forwardRenderList.size(); ++iRenderItem)
      {
        const RenderingItem& renderItem = forwardRenderList[iRenderItem];

        UpdatePerDrawData(camera, *renderItem.pWorldMat);
        
        const MaterialPass* pMaterialPass = renderItem.pMaterialPassInstance->getMaterialPass();
        if (pCachedMaterialPass != pMaterialPass)
        {
          pCachedMaterialPass = pMaterialPass;
          ApplyMaterialPass(pMaterialPass, context);
        }

        BindResources_ForwardColorPass(context, renderItem.pMaterialPassInstance);
        
        context->renderGeometry(renderItem.pGeometry);
      }  // end renderItems
    }  // end lights

    context->ExecuteAndReset(true);
    RenderContext::FreeContext(context);
  }
//---------------------------------------------------------------------------//
  void RenderingProcessForward::BindResources_ForwardColorPass(RenderContext* aContext, const MaterialPassInstance* aMaterial) const
  {
    aContext->SetConstantBuffer(myPerLightData.get(), 0);
    aContext->SetConstantBuffer(myPerDrawData.get(), 1);

    const uint32 kNumTextures = 3u;
    Descriptor textureDescriptors[kNumTextures];

    const Texture* const* readTextures = aMaterial->getReadTextures();
    for (uint32 i = 0u; i < kNumTextures; ++i)
    {
      textureDescriptors[i] = readTextures[i]->GetSrv();
    }

    aContext->SetMultipleResources(textureDescriptors, kNumTextures, (uint32)GpuDescriptorTypeFlags::BUFFER_TEXTURE_CONSTANT_BUFFER, 2);
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering