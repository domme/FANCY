#pragma once

#include <fancy_core/MathPrerequisites.h>

namespace Fancy {
  class Window;
  struct InputState;
}

class Camera;

//---------------------------------------------------------------------------//
  class CameraController
  {
  public:
    CameraController(Fancy::Window* aWindow, Camera* aCamera);
    ~CameraController();
    
    void Update(float aDeltaTime, const Fancy::InputState& anInputState);

    float myMoveSpeed;

  private:
    void UpdateFPSCamera(float aDeltaTime, const Fancy::InputState& anInputState);
    void UpdateTrackballCamera(float aDeltaTime, const Fancy::InputState& anInputState);

    Fancy::Window* myWindow;
    Camera* myCamera;

    glm::float2 myMouseSensitivity;
    glm::int2 myLastMousePos;
    glm::float3 myFocusPoint;
    float myFocusPointDistance;
  };
//---------------------------------------------------------------------------//

