#ifndef INCLUDE_SCENE_H
#define INCLUDE_SCENE_H

#include "FancyCorePrerequisites.h"

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
  // Forward declarations:
  class SceneNode;
  class SceneRenderDescription;
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  class Scene
  {
    public:
      Scene();
      ~Scene();

      void update();
      void collectSceneRenderData(SceneRenderDescription* pRenderDesc);

      
    private:
      std::shared_ptr<SceneNode> m_pRootNode;
  };
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Scene

#endif  // INCLUDE_SCENE_H
