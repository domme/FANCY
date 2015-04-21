//#include "CameraControllerComponent.h"
//#include "SceneNode.h"
//#include "InputManager.h"
//
//namespace Fancy { namespace Scene {
////---------------------------------------------------------------------------//
//  namespace Internal
//  {
//    glm::vec3 cameraMovement;
//    bool rightMouseButtonDown;
//  }
////---------------------------------------------------------------------------//
//
////---------------------------------------------------------------------------//
//  CameraControllerComponent::CameraControllerComponent( SceneNode* _pOwner ) : 
//    SceneNodeComponent(_pOwner)
//  {
//
//  }
////---------------------------------------------------------------------------//
//  CameraControllerComponent::~CameraControllerComponent()
//  {
//
//  }
////---------------------------------------------------------------------------//
//  void CameraControllerComponent::init()
//  {
//    ASSERT_M(m_pOwner->getComponent(_N(CameraComponent)) != nullptr, "The camera controller requires a camera");
//    m_pCamera = m_pOwner->getCameraComponentPtr();
//  }
////---------------------------------------------------------------------------//
//  void CameraControllerComponent::update()
//  {
//    CameraComponentPtr pCamera = m_pCamera.lock();
//    if (!pCamera) return;
//
//    
//  }
////---------------------------------------------------------------------------//
//} } // end of namespace Fancy::Scene