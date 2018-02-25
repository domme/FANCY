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
#include "Model.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  GraphicsWorld::GraphicsWorld()
  {
    myScene = std::make_unique<Scene::Scene>();
  }
//---------------------------------------------------------------------------//
  GraphicsWorld::~GraphicsWorld()
  {
    myScene.reset();

#define CHECK_UNIQUE_PTRS(aCollection, aName) \
    for (const auto& entry : aCollection) \
      ASSERT(entry.second.unique(), "Dangling reference found when trying to delete % cache", aName); 

    CHECK_UNIQUE_PTRS(myModelCache, "Model");
    myModelCache.clear();

    CHECK_UNIQUE_PTRS(mySubModelCache, "SubModel");
    mySubModelCache.clear();

    CHECK_UNIQUE_PTRS(myMaterialCache, "Material");
    myMaterialCache.clear();

#undef CHECK_UNIQUE_PTRS
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
  void GraphicsWorld::Startup() const
  {
    myScene->startup();
  }
//---------------------------------------------------------------------------//
  void GraphicsWorld::Tick(const Time& aClock) const
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
  SharedPtr<Geometry::Model> GraphicsWorld::CreateModel(const Geometry::ModelDesc& aDesc)
  {
    if (aDesc.IsEmpty())
      return nullptr;

    auto it = myModelCache.find(aDesc.GetHash());
    if (it != myModelCache.end())
      return it->second;

    SharedPtr<Geometry::Model> model(FANCY_NEW(Geometry::Model, MemoryCategory::GEOMETRY));
    model->SetFromDescription(aDesc, this);

    myModelCache.insert(std::make_pair(aDesc.GetHash(), model));
    return model;
  }
//---------------------------------------------------------------------------//
}