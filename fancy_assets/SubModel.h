#pragma once

#include <fancy_core/FancyCorePrerequisites.h>

namespace Fancy {
//---------------------------------------------------------------------------//
  class Mesh;
  class Material;
//---------------------------------------------------------------------------//
  class SubModel
  {
  public:
    SharedPtr<Material> myMaterial;
    SharedPtr<Mesh> myMesh;
  }; 
//---------------------------------------------------------------------------//
}