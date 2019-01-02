#pragma once

#include "ModelDesc.h"

#include <fancy_core/Ptr.h>

namespace Fancy {
  struct Material;
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
