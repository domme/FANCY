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
  class Material;
  struct MaterialDesc;
} }

namespace Fancy {
//---------------------------------------------------------------------------//
  class Time;
//---------------------------------------------------------------------------//
  class GraphicsWorld
  {
    public:
      GraphicsWorld();
      ~GraphicsWorld();

      Scene::SceneNode* Import(const std::string& aPath);
      void Startup() const;
      void Tick(const Time& aClock) const;

      Scene::Scene* GetScene() const { return myScene.get(); }
      
      SharedPtr<Geometry::SubModel> CreateSubModel(const Geometry::SubModelDesc& aDesc);
      SharedPtr<Rendering::Material> CreateMaterial(const Rendering::MaterialDesc& aDesc);
      SharedPtr<Geometry::Model> CreateModel(const Geometry::ModelDesc& aDesc);

    private:
      std::unique_ptr<Scene::Scene> myScene;

      std::map<uint64, SharedPtr<Rendering::Material>> myMaterialCache;
      std::map<uint64, SharedPtr<Geometry::SubModel>> mySubModelCache;
      std::map<uint64, SharedPtr<Geometry::Model>> myModelCache;
  };
//---------------------------------------------------------------------------//
}