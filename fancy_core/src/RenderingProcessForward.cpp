#include "RenderingProcessForward.h"
#include "Renderer.h"
#include "ShaderConstantsManager.h"
#include "Scene.h"
#include "EngineCommon.h"
#include "SceneRenderDescription.h"
#include "MaterialPass.h"
#include "LightComponent.h"
#include "GpuDataInterface.h"
#include "MaterialPassInstance.h"

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
    Scene::Scene* pScene = EngineCommon::getCurrentScene().get();
    Renderer& renderer = *EngineCommon::GetRenderer();

    Scene::SceneRenderDescription renderDesc;
    pScene->gatherRenderItems(&renderDesc);

    ShaderConstantsManager::bindBuffers(renderer.GetDefaultContext());

    ShaderConstantsManager::updateStage.myRenderContext = renderer.GetDefaultContext();
    ShaderConstantsManager::update(ConstantBufferType::PER_FRAME);

    ShaderConstantsManager::updateStage.pCamera = pScene->getActiveCamera();
    ShaderConstantsManager::update(ConstantBufferType::PER_CAMERA);

    const Scene::RenderingItemList& forwardRenderList = renderDesc.techniqueItemList[(uint32) Rendering::EMaterialPass::SOLID_FORWARD];
    // TODO: Sort based on material-pass

    const Scene::LightList& aLightList = pScene->getCachedLights();

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
          myGpuDataInterface->applyMaterialPass(pMaterialPass, &renderer);
        }

        myGpuDataInterface->applyMaterialPassInstance(renderItem.pMaterialPassInstance, &renderer);

        // TODO: Do this with RenderContexts
        // renderer.renderGeometry(renderItem.pGeometry);
      }  // end renderItems
    }  // end lights
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering