#ifndef INCLUDE_SCENEIMPORTER_H
#define INCLUDE_SCENEIMPORTER_H

#include "FancyCorePrerequisites.h"

namespace Fancy {
class GraphicsWorld;
}

namespace Fancy { namespace Scene {
  class Scene;
  class SceneNode;
} }

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  class DLLEXPORT SceneImporter 
  {
    public: 
      // static void _shaderTest();

      static void initLogger();
      static void destroyLogger();
      static bool importToSceneGraph(const std::string& _szImportPathRel, Scene::SceneNode* _pParentNode, GraphicsWorld* aWorld);
  };
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO

#endif  // INCLUDE_SCENEIMPORTER_H