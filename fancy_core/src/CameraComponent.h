#ifndef INCLUDE_CAMERACOMPONENT_H
#define INCLUDE_CAMERACOMPONENT_H

#include "SceneNodeComponent.h"

namespace Fancy { namespace Scene {
  //---------------------------------------------------------------------------//
  class DLLEXPORT CameraComponent : 
    public SceneNodeComponent, public BaseCreator<CameraComponent, SceneNode*>
  {
  public:
    CameraComponent(SceneNode* pOwner);
    virtual ~CameraComponent();

    virtual void update() override;
    virtual ObjectName getTypeName() override { return _N(Camera); }
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(CameraComponent)
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Scene

#endif  // INCLUDE_CAMERACOMPONENT_H