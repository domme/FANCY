#pragma once

#include "MaterialDesc.h"
#include <fancy_core/MeshDesc.h>

namespace Fancy
{
  struct ModelDesc
  {
    uint64 GetHash() const;
    MaterialDesc myMaterial;
    MeshDesc myMesh;
  };
}

