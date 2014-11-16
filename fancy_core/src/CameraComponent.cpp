#include "CameraComponent.h"
#include "SceneNode.h"
#include "TransformComponent.h"

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
  void CameraComponent::updateCameraInternal()
  {
    if (m_pCamera)
    {
      const glm::mat4& viewInverse = 
        m_pOwner->getTransformComponent()->getCachedWorldTransform();

      m_pCamera->setViewInv(viewInverse);
    }
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Scene