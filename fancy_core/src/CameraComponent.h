#ifndef INCLUDE_CAMERACOMPONENT_H
#define INCLUDE_CAMERACOMPONENT_H

#include "SceneNodeComponent.h"
#include "Camera.h"

namespace Fancy { namespace Scene {
  //---------------------------------------------------------------------------//
  class DLLEXPORT CameraComponent : 
    public SceneNodeComponent, public BaseCreator<CameraComponent, SceneNode*>
  {
  public:
    CameraComponent(SceneNode* pOwner);
    virtual ~CameraComponent();

    void setCamera(Camera* pCamera) {m_pCamera;}

    virtual void update() override;
    virtual void gatherRenderItems(SceneRenderDescription* pRenderDesc) override;
    virtual ObjectName getTypeName() override { return _N(Camera); }

  private:
    void updateCameraInternal();
    Camera* m_pCamera;
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(CameraComponent)
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Scene

#endif  // INCLUDE_CAMERACOMPONENT_H