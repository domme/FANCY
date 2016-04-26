#include "RenderingProcessForward.h"
#include "Renderer.h"
#include "ShaderConstantsManager.h"
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

namespace Fancy { namespace Rendering {

  GpuProgramPipeline ourDummyPipeline;
  Geometry::GeometryData ourFsQuadGeometry;

//---------------------------------------------------------------------------//
  RenderingProcessForward::RenderingProcessForward()
  {
    // Setup dummy stuff
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
    bufferParams.myUsageFlags = (uint32) GpuBufferUsage::VERTEX_BUFFER;
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
  RenderingProcessForward::~RenderingProcessForward()
  {

  }
//---------------------------------------------------------------------------//
  void RenderingProcessForward::startup()
  {
    // ShaderConstantsManager::update(ConstantBufferType::PER_LAUNCH);
  }
//---------------------------------------------------------------------------//
  void RenderingProcessForward::tick(float _dt)
  {
    Scene::Scene* pScene = Fancy::GetCurrentScene().get();
    Renderer& renderer = *Fancy::GetRenderer();
    
    RenderContext* context = RenderContext::AllocateContext();

    //context->setViewport(glm::uvec4(0, 0, 1280, 720));
    //context->setRenderTarget(renderer.GetBackbuffer(), 0u);
    //context->SetGpuProgramPipeline(&ourDummyPipeline);
    //
    //BlendState* blendState = BlendState::FindFromDesc(BlendStateDesc::GetDefaultSolid());
    //DepthStencilState* dsState = DepthStencilState::FindFromDesc(DepthStencilStateDesc::GetDefaultDepthNoStencil());
    //
    //context->setBlendState(*blendState);
    //context->setDepthStencilState(*dsState);
    //context->setCullMode(CullMode::NONE);
    //context->setFillMode(FillMode::SOLID);
    //context->setWindingOrder(WindingOrder::CCW);
    //
    //context->renderGeometry(&ourFsQuadGeometry);

    Scene::SceneRenderDescription renderDesc;
    pScene->gatherRenderItems(&renderDesc);

    ShaderConstantsManager::updateStage.myRenderContext = context;
    ShaderConstantsManager::update(ConstantBufferType::PER_FRAME);

    ShaderConstantsManager::updateStage.pCamera = pScene->getActiveCamera();
    ShaderConstantsManager::update(ConstantBufferType::PER_CAMERA);

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
      ShaderConstantsManager::updateStage.pLight = aLight;
      ShaderConstantsManager::update(ConstantBufferType::PER_LIGHT);

      ResourceBindingDataSource bindingSource;
      
      for (uint32 iRenderItem = 0u; iRenderItem < forwardRenderList.size(); ++iRenderItem)
      {
        const RenderingItem& renderItem = forwardRenderList[iRenderItem];

        ShaderConstantsManager::updateStage.pWorldMat = renderItem.pWorldMat;

        ShaderConstantsManager::updateStage.pMaterial = renderItem.pMaterialPassInstance;
        ShaderConstantsManager::update(ConstantBufferType::PER_MATERIAL);
        ShaderConstantsManager::update(ConstantBufferType::PER_OBJECT);

        const MaterialPass* pMaterialPass = renderItem.pMaterialPassInstance->getMaterialPass();
        if (pCachedMaterialPass != pMaterialPass)
        {
          pCachedMaterialPass = pMaterialPass;
          ApplyMaterialPass(pMaterialPass, context);
        }

        bindingSource.myMaterial = renderItem.pMaterialPassInstance;
        ResourceBinding::BindResources_ForwardColorPass(context, bindingSource);
        
        context->renderGeometry(renderItem.pGeometry);
      }  // end renderItems
    }  // end lights

    context->ExecuteAndReset(true);
    RenderContext::FreeContext(context);
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering