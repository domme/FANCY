#pragma once
#include "MaterialDesc.h"

#include <fancy_core/MeshDesc.h>

namespace Fancy
{
  struct ModelDesc
  {
    MaterialDesc myMaterial;
    MeshDesc myMesh;
  };
}

