#include "fancy_core_precompile.h"

#include "Camera.h"
#include "MathUtil.h"
#include "FancyCoreDefines.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  Camera::Camera()
    : myIsOrtho(false)
    , myFovDeg(0.0f)
    , myFar(0.0f)
    , myNear(0.0f)
    , myWidth(0.0f)
    , myHeight(0.0f)
    , myLeft(0.0f)
    , myRight(0.0f)
    , myBottom(0.0f)
    , myTop(0.0f)
  {
  }
//---------------------------------------------------------------------------//
  Camera::~Camera()
  {
  }
//---------------------------------------------------------------------------//
  void Camera::UpdateProjection()
  {
    if (!myIsOrtho)
      myProjection = Fancy::MathUtil::perspectiveFov(myFovDeg, myWidth, myHeight, myNear, myFar);
    else
      myProjection = glm::ortho(myLeft, myRight, myBottom, myTop, myNear, myFar);

    myViewProj = myProjection * myView;
    UpdateFrustumPlanes();
  }
//---------------------------------------------------------------------------//
  void Camera::UpdateView()
  {
    myViewInv = glm::translate(myPosition) * glm::toMat4(myOrientation);
    myView = glm::inverse(myViewInv);
    myViewProj = myProjection * myView;
  }
  //---------------------------------------------------------------------------//
  //3----2
  //|    |
  //0----1
  void Camera::GetVerticesOnNearPlane(eastl::fixed_vector<glm::float3, 4>& someVerticesOut)
  {
    glm::float3 verts[4] = {
      { -1.0f, -1.0f, 0.0f }, { 1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f }, { -1.0f, 1.0f, 0.0f }
    };

    glm::float4x4 viewProjInv = glm::inverse(myViewProj);

    for (const glm::float3& v : verts)
    {
      glm::float4 vWorld = viewProjInv * glm::float4(v, 1.0f);
      float div = 1.0f / vWorld.w;
      vWorld.x *= div;
      vWorld.y *= div;
      vWorld.z *= div;

      someVerticesOut.push_back(vWorld);
    }
  }
  //---------------------------------------------------------------------------//
  void Camera::UpdateFrustumPlanes()
  {
    //If the camera's projection matrix is an orthogonal projection, the frustum planes have to be derived 
    //from the general projection matrix
    if (myIsOrtho)
    {
      glm::mat4 projT = glm::transpose(myProjection);

      myFrustumPlanesVS[PLANE_NEAR] = projT[3] + projT[2];
      myFrustumPlanesVS[PLANE_FAR] = projT[3] - projT[2];
      myFrustumPlanesVS[PLANE_LEFT] = projT[3] + projT[0];
      myFrustumPlanesVS[PLANE_RIGHT] = projT[3] - projT[0];
      myFrustumPlanesVS[PLANE_BOTTOM] = projT[3] + projT[1];
      myFrustumPlanesVS[PLANE_TOP] = projT[3] - projT[1];

      //The normals in the plane-vectors (N.x, N.y, N.z, D) have to be normalized
      glm::vec3 v3N;
      for (uint i = 0; i < 6; ++i)
      {
        v3N = glm::normalize(glm::vec3(myFrustumPlanesVS[i]));
        myFrustumPlanesVS[i].x = v3N.x;
        myFrustumPlanesVS[i].y = v3N.y;
        myFrustumPlanesVS[i].z = v3N.z;
      }
    }

    //If the camera's projection matrix is a perpsective projection, the view-space frustum planes can be
    //determined by the proj-parameters of the camera (more efficient this way)
    else
    {
      float aspect = myWidth / myHeight;
      float fFovHor2 = glm::atan(aspect * glm::tan(glm::radians(myFovDeg) / 2.0f));
      float focalLength = 1.0f / glm::tan(fFovHor2);

      float fe1 = glm::sqrt(focalLength * focalLength + 1);
      float fea = glm::sqrt(focalLength * focalLength + aspect * aspect);

      myFrustumPlanesVS[PLANE_NEAR] = glm::vec4(0.0f, 0.0f, -1.0f, -myNear);
      myFrustumPlanesVS[PLANE_FAR] = glm::vec4(0.0f, 0.0f, 1.0f, myFar);
      myFrustumPlanesVS[PLANE_LEFT] = glm::vec4(focalLength / fe1, 0.0f, -1.0f / fe1, 0.0f);
      myFrustumPlanesVS[PLANE_RIGHT] = glm::vec4(-focalLength / fe1, 0.0f, -1.0f / fe1, 0.0f);
      myFrustumPlanesVS[PLANE_BOTTOM] = glm::vec4(0.0f, focalLength / fea, -aspect / fea, 0.0f);
      myFrustumPlanesVS[PLANE_TOP] = glm::vec4(0.0f, -focalLength / fea, -aspect / fea, 0.0f);
    }
  }
//---------------------------------------------------------------------------//
}
