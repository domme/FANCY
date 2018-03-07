#pragma once

#include <fancy_core/FancyCorePrerequisites.h>
#include "ModelDesc.h"

namespace Fancy {
  class Material;
  class Mesh;

  //---------------------------------------------------------------------------//
  struct Model
  {
    ModelDesc GetDescription() const;

    SharedPtr<Mesh> myMesh;
    SharedPtr<Material> myMaterial;
  };
//---------------------------------------------------------------------------//
}
