#pragma once

#include "FancyCorePrerequisites.h"
#include "ScopedPtr.h"

namespace Fancy { namespace Scene {
  class SceneNode;
  class Scene;
} }

namespace Fancy { namespace Geometry {
  class SubModel;
  class Model;
  struct ModelDesc;
  struct SubModelDesc;
  struct MeshDesc;
} }

namespace Fancy { namespace Rendering {
  class MaterialPassInstance;
  class Material;
  class MaterialPass;
  struct MaterialPassDesc;
  struct MaterialDesc;
  struct MaterialPassInstanceDesc;
} }

namespace Fancy {
//---------------------------------------------------------------------------//
  class Time;
//---------------------------------------------------------------------------//
  class DLLEXPORT GraphicsWorld
  {
    public:
      GraphicsWorld();

      Scene::SceneNode* Import(const std::string& aPath);
      void Startup();
      void Tick(const Time& aClock);

      Scene::Scene* GetScene() const { return myScene.Get(); }
      
      SharedPtr<Geometry::SubModel> CreateSubModel(const Geometry::SubModelDesc& aDesc);
      SharedPtr<Rendering::Material> CreateMaterial(const Rendering::MaterialDesc& aDesc);
      SharedPtr<Rendering::MaterialPassInstance> CreateMaterialPassInstance(const Rendering::MaterialPassInstanceDesc& aDesc);
      SharedPtr<Rendering::MaterialPass> CreateMaterialPass(const Rendering::MaterialPassDesc& aDesc);
      SharedPtr<Geometry::Model> CreateModel(const Geometry::ModelDesc& aDesc);

    private:
      ScopedPtr<Scene::Scene> myScene;

      std::map<uint64, SharedPtr<Rendering::Material>> myMaterialCache;
      std::map<uint64, SharedPtr<Geometry::SubModel>> mySubModelCache;
      std::map<uint64, SharedPtr<Geometry::Model>> myModelCache;
      std::map<uint64, SharedPtr<Rendering::MaterialPassInstance>> myMaterialPassInstanceCache;
      std::map<uint64, SharedPtr<Rendering::MaterialPass>> myMaterialPassCache;
  };
//---------------------------------------------------------------------------//
}