#ifndef INCLUDE_CAMERACONTROLLERCOMPONENT_H
#define INCLUDE_CAMERACONTROLLERCOMPONENT_H

#include "SceneNodeComponent.h"
#include "CameraComponent.h"
#include "FixedArray.h"

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
  class DLLEXPORT CameraControllerComponent : 
    public SceneNodeComponent, public BaseCreator<CameraControllerComponent, SceneNode*>
  {
    public:
      CameraControllerComponent(SceneNode* _pOwner);
      virtual ~CameraControllerComponent();

      virtual void init() override;
      virtual void update() override;
	  virtual void gatherRenderItems(SceneRenderDescription* pRenderDesc) override {};
      virtual ObjectName getTypeName() override { return _N(CameraController); }

    private:
      CameraComponentWeakPtr m_pCamera;
  };
  //---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(CameraControllerComponent)
  //---------------------------------------------------------------------------//
} } // end of namespace Fancy::Scene

#endif  // INCLUDE_CAMERACONTROLLERCOMPONENT_H