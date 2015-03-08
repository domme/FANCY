#include "CameraComponent.h"
#include "SceneNode.h"

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
  CameraComponent::CameraComponent(SceneNode* pOwner)
    : SceneNodeComponent(pOwner)
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
    const glm::mat4& viewInverse = m_pOwner->getTransform().getCachedWorld();

    m_camera.setViewInv(viewInverse);
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Scene