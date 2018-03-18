#include "CameraController.h"
#include "fancy_core/Input.h"
#include "Camera.h"

using namespace Fancy;

CameraController::CameraController(Window* aWindow, Camera* aCamera)
  : myMoveSpeed(30.0f) 
  , myWindow(aWindow)
  , myCamera(aCamera)
{

}

CameraController::~CameraController()
{
}

void CameraController::Update(float aDeltaTime, const Fancy::InputState& anInputState)
{
  const float movementSpeed = myMoveSpeed * aDeltaTime;
  if (anInputState.myKeyState['w'])
    myCamera->myPosition.z += movementSpeed;
  if (anInputState.myKeyState['s'])
    myCamera->myPosition.z -= movementSpeed;
  if (anInputState.myKeyState['a'])
    myCamera->myPosition.x -= movementSpeed;
  if (anInputState.myKeyState['d'])
    myCamera->myPosition.x += movementSpeed;
  if (anInputState.myKeyState['q'])
    myCamera->myPosition.y -= movementSpeed;
  if (anInputState.myKeyState['e'])
    myCamera->myPosition.y += movementSpeed;

  if (anInputState.myMouseBtnMask & InputState::MOUSE_BTN_RIGHT)
  {
    float angleX = glm::radians((float)anInputState.myMouseDelta.y);
    float angleY = glm::radians((float)anInputState.myMouseDelta.x);
    
    glm::quat rotation = glm::angleAxis(angleY, glm::float3(1.0f, 0.0f, 0.0f)) * glm::angleAxis(angleY, glm::float3(0.0f, 1.0f, 0.0f));
    myCamera->myOrientation = rotation;
  }

  myCamera->UpdateView();
}
