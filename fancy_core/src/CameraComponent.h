#ifndef INCLUDE_CAMERACOMPONENT_H
#define INCLUDE_CAMERACOMPONENT_H

#include "SceneNodeComponent.h"

namespace Fancy { namespace Scene {
  //---------------------------------------------------------------------------//
  class DLLEXPORT CameraComponent : public SceneNodeComponent
  {
  public:
    CameraComponent();
    virtual ~CameraComponent();

    virtual void update(float fDt) = 0;
  };
  //---------------------------------------------------------------------------//
} } // end of namespace Fancy::Scene

#endif  // INCLUDE_CAMERACOMPONENT_H