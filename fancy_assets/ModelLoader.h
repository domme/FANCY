#pragma once
#include <fancy_core/FancyCorePrerequisites.h>

namespace Fancy {
  //---------------------------------------------------------------------------//
  struct Model;
  class AssetManager;
//---------------------------------------------------------------------------//
  namespace ModelLoader
  {
    enum ImportOptions
    {
      NONE = 0,
      CALC_TANGENT_SPACE      = 1 << 0,
      TRIANGULATE             = 1 << 1,
      JOIN_IDENTICAL_VERTICES = 1 << 2,
      SPLIT_BY_PRIMITIVE_TYPE = 1 << 3,
      INSTANTIATE_DUPLICATES  = 1 << 4,
      ALL = ~0u
    };

    struct Scene
    {
      void Clear() { myTransforms.clear(); myModels.clear(); }
      DynamicArray<glm::mat4> myTransforms;
      DynamicArray<SharedPtr<Model>> myModels;
    };

    bool LoadFromFile(const char* aPath, AssetManager& aStorage, Scene& aSceneOut, ImportOptions someImportOptions = ALL);
  }
//---------------------------------------------------------------------------//
}

