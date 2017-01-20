#include "GraphicsWorld.h"
#include "MeshDesc.h"
#include "BinaryCache.h"
#include "Mesh.h"
#include "GeometryData.h"
#include "Scene.h"
#include "SceneImporter.h"
#include "SceneNode.h"
#include "TimeManager.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  GraphicsWorld::GraphicsWorld()
  {
    myScene = new Scene::Scene();
  }
//---------------------------------------------------------------------------//
  Scene::SceneNode* GraphicsWorld::Import(const std::string& aPath)
  {
    IO::SceneImporter importer(*this);
    Scene::SceneNode* node = myScene->getRootNode()->createChildNode();
    if (!importer.importToSceneGraph(aPath, node))
      return nullptr;

    return node;
  }
//---------------------------------------------------------------------------//
  void GraphicsWorld::Startup()
  {
    myScene->startup();
  }
//---------------------------------------------------------------------------//
  void GraphicsWorld::Tick(const Time& aClock)
  {
    const float deltaTime = aClock.GetDelta();
    myScene->update(deltaTime);
  }
//---------------------------------------------------------------------------//
}
