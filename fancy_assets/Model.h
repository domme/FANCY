#pragma once

#include <fancy_core/FancyCorePrerequisites.h>
#include "ModelDesc.h"

namespace Fancy {
  class Material;
  class Mesh;

  //---------------------------------------------------------------------------//
  struct Model
  {
    Model();
    ModelDesc GetDescription() const;

    Mesh* myMesh;
    Material* myMaterial;
  };
//---------------------------------------------------------------------------//
}
