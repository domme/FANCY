#include "CameraComponent.h"
#include "SceneNode.h"

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
  CameraComponent::CameraComponent(SceneNode* pOwner)
    : SceneNodeComponent(pOwner),
    m_pCamera(nullptr)
  {

  }
//---------------------------------------------------------------------------//
  CameraComponent::~CameraComponent()
  {

  }
//---------------------------------------------------------------------------//
  void CameraComponent::update()
  {
    updateCameraInternal();
  }
//---------------------------------------------------------------------------//
  void CameraComponent::gatherRenderItems( SceneRenderDescription* pRenderDesc )
  {

  }
//---------------------------------------------------------------------------//
  void CameraComponent::updateCameraInternal()
  {
    if (m_pCamera)
    {
      const glm::mat4& viewInverse = 
        m_pOwner->getTransform().getCachedWorld();

      m_pCamera->setViewInv(viewInverse);
    }
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Scene