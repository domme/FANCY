#ifndef INCLUDE_SCENE_H
#define INCLUDE_SCENE_H

#include "FancyCorePrerequisites.h"

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
  // Forward declarations:
  class SceneNode;
  class CameraComponent;
  class SceneRenderDescription;
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  class DLLEXPORT Scene
  {
    public:
      Scene();
      ~Scene();

      SceneNode* getRootNode() {return m_pRootNode.get();}

      void update();
      void gatherRenderItems(SceneRenderDescription* pRenderDesc);
      void setActiveCamera(const std::shared_ptr<CameraComponent>& _pCamera) {m_pActiveCamera = _pCamera;}
      CameraComponent* getActiveCamera() {return m_pActiveCamera.lock().get();}
      
    private:
      std::shared_ptr<SceneNode> m_pRootNode;
      std::weak_ptr<CameraComponent> m_pActiveCamera;
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(Scene)
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Scene

#endif  // INCLUDE_SCENE_H
