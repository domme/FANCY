#pragma once

#include "MathIncludes.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct InputState;
  class Camera;
//---------------------------------------------------------------------------//
  class CameraController
  {
  public:
    CameraController(Camera* aCamera);
    ~CameraController();

    void Update(float aDeltaTime, const InputState& anInputState);

    float myMoveSpeed;

  private:
    enum class Mode
    {
      FPS,
      TRACKBALL
    };

    void UpdateFPSCamera(float aDeltaTime, const Fancy::InputState& anInputState);
    void UpdateTrackballCamera(float aDeltaTime, const Fancy::InputState& anInputState);

    Camera* myCamera;

    glm::float2 myMouseSensitivity;
    glm::int2 myLastMousePos;
    glm::float3 myFocusPoint;
    float myFocusPointDistance;
    Mode myLastMode;
  };
//---------------------------------------------------------------------------//
}