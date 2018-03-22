#include "CameraController.h"
#include "fancy_core/Input.h"
#include "Camera.h"

using namespace Fancy;

CameraController::CameraController(Window* aWindow, Camera* aCamera)
  : myMoveSpeed(30.0f) 
  , myWindow(aWindow)
  , myCamera(aCamera)
  , myLastMousePos(0,0)
  , myFocusPoint(0.0f)
  , myFocusPointDistance(10.0f)
{
  
}

CameraController::~CameraController()
{
}

void CameraController::Update(float aDeltaTime, const Fancy::InputState& anInputState)
{
  if (anInputState.myModifierKeyMask & InputState::MOD_KEY_ALT || anInputState.myModifierKeyMask & InputState::MOD_KEY_SHIFT)
  {
    UpdateTrackballCamera(aDeltaTime, anInputState);
  }
  else if(anInputState.myMouseBtnMask & InputState::MOUSE_BTN_RIGHT)
  {
    UpdateFPSCamera(aDeltaTime, anInputState);
  }

  myCamera->UpdateView();
  myLastMousePos = anInputState.myMousePos;
}

void CameraController::UpdateFPSCamera(float aDeltaTime, const Fancy::InputState& anInputState)
{
  glm::float3 camForward = myCamera->myOrientation * glm::float3(0.0f, 0.0f, 1.0f);
  glm::float3 camSide = myCamera->myOrientation * glm::float3(1.0f, 0.0f, 0.0f);

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

  glm::float3 cameraEulerAngles = glm::eulerAngles(myCamera->myOrientation);
  cameraEulerAngles.y = glm::clamp(cameraEulerAngles.y + glm::radians((float)mouseDelta.x) * 0.25f, -glm::pi<float>(), glm::pi<float>());
  cameraEulerAngles.x = glm::clamp(cameraEulerAngles.x + glm::radians((float)mouseDelta.y) * 0.25f, -glm::pi<float>(), glm::pi<float>());

  myCamera->myOrientation = glm::quat(cameraEulerAngles);

  myFocusPoint = myCamera->myPosition + myCamera->myOrientation * glm::float3(0.0f, 0.0f, myFocusPointDistance);
}

void CameraController::UpdateTrackballCamera(float aDeltaTime, const Fancy::InputState& anInputState)
{
  /*if (anInputState.myMouseBtnMask & InputState::MOUSE_BTN_RIGHT)
  {
    glm::ivec2 mouseDelta = anInputState.myMousePos - myLastMousePos;
    float yaw = glm::radians((float)mouseDelta.x) * 0.25f;
    float pitch = glm::radians((float)mouseDelta.y) * 0.25f;
    glm::quat yawQuat(glm::float3(0.0f, yaw, 0.0f));
    glm::quat pitchQuat(glm::float3(pitch, 0.0f, 0.0f));

    myCamera->myPosition = glm::rotate(pitchQuat, glm::rotate(yawQuat, myCamera->myPosition));

    glm::float3 forward = glm::normalize(myFocusPoint - myCamera->myPosition);
    glm::float3 side = glm::normalize(glm::cross(glm::float3(0,1,0), forward));
    glm::float3 up = glm::normalize(glm::cross(forward, side));
    glm::float3x3 rot(side, up, forward);
    myCamera->myOrientation = glm::quat_cast(rot);
  }
  */
}
