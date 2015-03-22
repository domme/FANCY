#ifndef INCLUDE_CAMERACONTROLLER_H_
#define INCLUDE_CAMERACONTROLLER_H_

#include <CameraComponent.h>
#include <SceneNode.h>

class CameraController
{
  public:
    ~CameraController();
    CameraController();

  private:
    
    Fancy::Scene::CameraComponent* m_pCameraComponent;
};

#endif // INCLUDE_CAMERACONTROLLER_H_
