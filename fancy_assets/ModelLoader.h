#pragma once
#include <fancy_core/FancyCorePrerequisites.h>
#include "MaterialDesc.h"
#include <fancy_core/Callback.h>
#include "Model.h"
#include "ModelDesc.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class Mesh;
  class Material;
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

    struct LoadResult
    {
      DynamicArray<glm::mat4> myTransforms;
      DynamicArray<SharedPtr<Model>> myModels;
    };

    bool LoadFromFile(const char* aPath, LoadResult& aResultOut, ImportOptions someImportOptions = ALL);
  }
}

