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
  class DLLEXPORT Scene
  {
    public:
      Scene();
      ~Scene();

      SceneNode* getRootNode() {return m_pRootNode.get();}

      void update();
      void gatherRenderItems(SceneRenderDescription* pRenderDesc);
            
    private:
      std::shared_ptr<SceneNode> m_pRootNode;
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(Scene)
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Scene

#endif  // INCLUDE_SCENE_H
