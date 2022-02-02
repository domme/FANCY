#include "fancy_core_precompile.h"

#include "CameraController.h"
#include "Input.h"
#include "Camera.h"
#include "MathIncludes.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  CameraController::CameraController(Camera* aCamera)
    : myMoveSpeed(30.0f)
    , myCamera(aCamera)
    , myMouseSensitivity(0.1f, 0.1f)
    , myLastMousePos(0, 0)
    , myFocusPoint(0.0f)
    , myFocusPointDistance(10.0f)
  {

  }
//---------------------------------------------------------------------------//
  CameraController::~CameraController()
  {
  }
//---------------------------------------------------------------------------//
  void CameraController::Update(float aDeltaTime, const Fancy::InputState& anInputState)
  {
    Mode currentMode = Mode::FPS;
    if (anInputState.myModifierKeyMask & InputState::MOD_KEY_ALT || anInputState.myModifierKeyMask & InputState::MOD_KEY_SHIFT)
      currentMode = Mode::TRACKBALL;

    if (currentMode == Mode::TRACKBALL)
    {
      UpdateTrackballCamera(aDeltaTime, anInputState);
    }
    else
    {
      UpdateFPSCamera(aDeltaTime, anInputState);
    }

    myCamera->UpdateView();
    myLastMousePos = anInputState.myMousePos;
    myLastMode = currentMode;
  }
//---------------------------------------------------------------------------//
  void CameraController::UpdateFPSCamera(float aDeltaTime, const Fancy::InputState& anInputState)
  {
    if (!(anInputState.myMouseBtnMask & InputState::MOUSE_BTN_RIGHT))
      return;

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

    glm::int2 mouseDelta = anInputState.myMousePos - myLastMousePos;

    float pitch = glm::radians((float)mouseDelta.y) * myMouseSensitivity.y;
    float yaw = glm::radians((float)mouseDelta.x) * myMouseSensitivity.x;
    glm::quat pitchQuat = glm::quat(glm::float3(pitch, 0.0f, 0.0f));
    glm::quat yawQuat = glm::quat(glm::float3(0.0f, yaw, 0.0f));
    myCamera->myOrientation = yawQuat * myCamera->myOrientation * pitchQuat;

    myFocusPoint = myCamera->myPosition + myCamera->myOrientation * glm::float3(0.0f, 0.0f, myFocusPointDistance);
  }

  void CameraController::UpdateTrackballCamera(float aDeltaTime, const Fancy::InputState& anInputState)
  {
    if (anInputState.myMouseBtnMask & InputState::MOUSE_BTN_LEFT)
    {
      glm::ivec2 mouseDelta = anInputState.myMousePos - myLastMousePos;
      float pitch = glm::radians((float)mouseDelta.y) * myMouseSensitivity.y;
      float yaw = glm::radians((float)mouseDelta.x) * myMouseSensitivity.x;

      glm::float3 centerToCamDir = glm::normalize(myCamera->myPosition - myFocusPoint);
      if (centerToCamDir.y < -0.9)
        pitch = glm::max<float>(pitch, 0.0f);
      else if (centerToCamDir.y > 0.9)
        pitch = glm::min<float>(pitch, 0.0f);

      glm::quat pitchQuat(glm::float3(pitch, 0.0f, 0.0f));
      glm::quat yawQuat(glm::float3(0.0f, yaw, 0.0f));
      myCamera->myOrientation = yawQuat * myCamera->myOrientation * pitchQuat;
      myCamera->myPosition = myFocusPoint + (myCamera->myOrientation * glm::float3(0.0f, 0.0f, -1.0f)) * myFocusPointDistance;
    }
  }
//---------------------------------------------------------------------------//
}

