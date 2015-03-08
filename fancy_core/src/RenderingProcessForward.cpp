#include "RenderingProcessForward.h"
#include "Renderer.h"
#include "ShaderConstantsManager.h"
#include "Scene.h"
#include "EngineCommon.h"
#include "CameraComponent.h"
#include "SceneRenderDescription.h"
#include "MaterialPass.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  RenderingProcessForward::RenderingProcessForward()
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
    Renderer& renderer = Renderer::getInstance();

    Scene::SceneRenderDescription renderDesc;
    pScene->gatherRenderItems(&renderDesc);

    ShaderConstantsManager::bindBuffers(&renderer);

    ShaderConstantsManager::updateStage.pRenderer = &renderer;
    ShaderConstantsManager::update(ConstantBufferType::PER_FRAME);

    ShaderConstantsManager::updateStage.pCamera = pScene->getActiveCamera()->getCamera();
    ShaderConstantsManager::update(ConstantBufferType::PER_CAMERA);

    const Scene::RenderingItemList& forwardRenderList = renderDesc.techniqueItemList[(uint32) Rendering::EMaterialPass::SOLID_FORWARD];
    // TODO: Sort based on material-pass

    const MaterialPass* pCachedMaterialPass = nullptr;
    for (uint32 i = 0u; i < forwardRenderList.size(); ++i)
    {
      const RenderingItem& renderItem = forwardRenderList[i];
      
      ShaderConstantsManager::updateStage.pWorldMat = renderItem.pWorldMat;

      ShaderConstantsManager::updateStage.pMaterial = renderItem.pMaterialPassInstance;
      ShaderConstantsManager::update(ConstantBufferType::PER_MATERIAL);
      ShaderConstantsManager::update(ConstantBufferType::PER_OBJECT);

      const MaterialPass* pMaterialPass = renderItem.pMaterialPassInstance->getMaterialPass();
      if (pCachedMaterialPass != pMaterialPass)
      {
        pCachedMaterialPass = pMaterialPass;
        applyMaterialPass(pMaterialPass, &renderer);
      }

      applyMaterialPassInstance(renderItem.pMaterialPassInstance, &renderer);

      renderer.renderGeometry(renderItem.pGeometry);
    }
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering