#include "RenderingProcessForward.h"
#include "Renderer.h"
#include "ShaderConstantsManager.h"
#include "Scene.h"
#include "Fancy.h"
#include "SceneRenderDescription.h"
#include "MaterialPass.h"
#include "LightComponent.h"
#include "GpuDataInterface.h"
#include "MaterialPassInstance.h"
#include "RenderContext.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  RenderingProcessForward::RenderingProcessForward() :
    myGpuDataInterface(new GpuDataInterface)
  {

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

    Scene::SceneRenderDescription renderDesc;
    pScene->gatherRenderItems(&renderDesc);

    ShaderConstantsManager::bindBuffers(context);

    ShaderConstantsManager::updateStage.myRenderContext = context;
    ShaderConstantsManager::update(ConstantBufferType::PER_FRAME);

    ShaderConstantsManager::updateStage.pCamera = pScene->getActiveCamera();
    ShaderConstantsManager::update(ConstantBufferType::PER_CAMERA);

    context->setRenderTarget(renderer.GetBackbuffer(), 0u);

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
          myGpuDataInterface->applyMaterialPass(pMaterialPass, context);
        }

        myGpuDataInterface->applyMaterialPassInstance(renderItem.pMaterialPassInstance, context);

        context->renderGeometry(renderItem.pGeometry);

        // TODO: Do this with RenderContexts
        // renderer.renderGeometry(renderItem.pGeometry);
      }  // end renderItems
    }  // end lights

    context->ExecuteAndReset(true);
    RenderContext::FreeContext(context);
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering