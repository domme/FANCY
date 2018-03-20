#pragma once

#include <glm/glm.hpp>

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

    glm::ivec2 myLastMousePos;
  };
//---------------------------------------------------------------------------//

