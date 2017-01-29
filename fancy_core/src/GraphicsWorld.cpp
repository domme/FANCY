#include "GraphicsWorld.h"
#include "MeshDesc.h"
#include "BinaryCache.h"
#include "Mesh.h"
#include "GeometryData.h"
#include "Scene.h"
#include "SceneImporter.h"
#include "SceneNode.h"
#include "TimeManager.h"
#include "SubModel.h"
#include "MaterialPassInstance.h"

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
  SharedPtr<Geometry::SubModel> GraphicsWorld::CreateSubModel(const Geometry::SubModelDesc& aDesc)
  {
    if (aDesc.IsEmpty())
      return nullptr;

    auto it = mySubModelCache.find(aDesc.GetHash());
    if (it != mySubModelCache.end())
      return it->second;

    SharedPtr<Geometry::SubModel> subModel(FANCY_NEW(Geometry::SubModel, MemoryCategory::GEOMETRY));
    subModel->SetFromDescription(aDesc, this);

    mySubModelCache.insert(std::make_pair(aDesc.GetHash(), subModel));
    return subModel;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Rendering::Material> GraphicsWorld::CreateMaterial(const Rendering::MaterialDesc& aDesc)
  {
    if (aDesc.IsEmpty())
      return nullptr;

    auto it = myMaterialCache.find(aDesc.GetHash());
    if (it != myMaterialCache.end())
      return it->second;

    SharedPtr<Rendering::Material> material(FANCY_NEW(Rendering::Material, MemoryCategory::MATERIALS));
    material->SetFromDescription(aDesc, this);

    myMaterialCache.insert(std::make_pair(aDesc.GetHash(), material));
    return material;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Rendering::MaterialPassInstance> GraphicsWorld::CreateMaterialPassInstance(const Rendering::MaterialPassInstanceDesc& aDesc)
  {
    if (aDesc.IsEmpty())
      return nullptr;

    auto it = myMaterialPassInstanceCache.find(aDesc.GetHash());
    if (it != myMaterialPassInstanceCache.end())
      return it->second;

    SharedPtr<Rendering::MaterialPassInstance> materialPassInstance(FANCY_NEW(Rendering::MaterialPassInstance, MemoryCategory::MATERIALS));
    materialPassInstance->SetFromDescription(aDesc, this);

    myMaterialPassInstanceCache.insert(std::make_pair(aDesc.GetHash(), materialPassInstance));
    return materialPassInstance;
  }
//---------------------------------------------------------------------------//
}
