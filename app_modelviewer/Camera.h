#pragma once

#include <fancy_core/MathPrerequisites.h>

class Camera
{
public:
  Camera();
  ~Camera();

  void UpdateProjection();
  void UpdateView();
  
  enum FrustumPlane
  {
    PLANE_LEFT = 0,
    PLANE_RIGHT,
    PLANE_BOTTOM,
    PLANE_TOP,
    PLANE_NEAR,
    PLANE_FAR,
    PLANE_NUM
  };

  glm::vec3 myPosition;
  glm::quat myOrientation;

  bool myIsOrtho;
  float myFovDeg;
  float myFar;
  float myNear;
  float myWidth;
  float myHeight;

  float myLeft;
  float myRight;
  float myBottom;
  float myTop;

  glm::mat4	myView;
  glm::mat4 myViewInv;
  glm::mat4	myProjection;
  glm::mat4	myViewProj;
  glm::vec4	myFrustumPlanesVS[6];

private:
  void UpdateFrustumPlanes();
};

