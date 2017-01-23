#pragma once

#include "FancyCorePrerequisites.h"
#include "ScopedPtr.h"

namespace Fancy { namespace Scene {
  class SceneNode;
  class Scene;
} }

namespace Fancy { namespace Geometry {
  struct ModelDesc;
  struct SubModelDesc;
  struct MeshDesc;
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

      // SharedPtr<Geometry::Model> CreateModel(const Geometry::ModelDesc& aDesc);

    private:
      ScopedPtr<Scene::Scene> myScene;

      std::map<uint64, SharedPtr<Geometry::SubModel>> mySubModelCache;

      
  };
//---------------------------------------------------------------------------//
}