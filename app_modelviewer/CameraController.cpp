#include "CameraController.h"
#include "fancy_core/Input.h"
#include "Camera.h"

using namespace Fancy;

CameraController::CameraController(Window* aWindow, Camera* aCamera)
  : myMoveSpeed(30.0f) 
  , myWindow(aWindow)
  , myCamera(aCamera)
  , myLastMousePos(0,0)
{
  
}

CameraController::~CameraController()
{
}

void CameraController::Update(float aDeltaTime, const Fancy::InputState& anInputState)
{
  glm::float3 camForward = myCamera->myOrientation * glm::float3(0.0f, 0.0f, 1.0f);
  glm::float3 camSide = myCamera->myOrientation * glm::float3(1.0f, 0.0f, 0.0f);
  glm::float3 camUp = myCamera->myOrientation * glm::float3(0.0f, 1.0f, 0.0f);

  const float movementSpeed = myMoveSpeed * aDeltaTime;
  if (anInputState.myKeyState['w'])
    myCamera->myPosition += camForward * movementSpeed;
  if (anInputState.myKeyState['s'])
    myCamera->myPosition -= camForward * movementSpeed;
  if (anInputState.myKeyState['a'])
    myCamera->myPosition -= camSide * movementSpeed;
  if (anInputState.myKeyState['d'])
    myCamera->myPosition += camSide * movementSpeed;
  if (anInputState.myKeyState['q'])
    myCamera->myPosition.y -= movementSpeed;
  if (anInputState.myKeyState['e'])
    myCamera->myPosition.y += movementSpeed;

  glm::ivec2 mouseDelta = anInputState.myMousePos - myLastMousePos;

  if (anInputState.myMouseBtnMask & InputState::MOUSE_BTN_RIGHT)
  {
    float angleX = glm::radians((float)mouseDelta.y) * 0.25f;
    float angleY = glm::radians((float)mouseDelta.x) * 0.25f;

    LOG_DEBUG("MouseDelta x: %", mouseDelta.x);
    LOG_DEBUG("MouseDelta y: %", mouseDelta.y);
    
    glm::quat rotation = glm::angleAxis(angleX, glm::float3(1.0f, 0.0f, 0.0)) * glm::angleAxis(angleY, camUp);
    myCamera->myOrientation *= rotation;
  }

  myCamera->UpdateView();

  myLastMousePos = anInputState.myMousePos;
}
