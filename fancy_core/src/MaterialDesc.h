#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "MaterialPassInstanceDesc.h"
#include "FixedArray.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  enum class EMaterialParameterSemantic
  {
    DIFFUSE_REFLECTIVITY = 0,
    SPECULAR_REFLECTIVITY,
    SPECULAR_POWER,
    OPACITY,

    NUM
  };
//---------------------------------------------------------------------------//
  struct MaterialDesc
  {
    float myParameters [(uint32)EMaterialParameterSemantic::NUM];
    MaterialPassInstanceDesc myPasses [(uint32)EMaterialPass::NUM];

    MaterialDesc();
    bool operator==(const MaterialDesc& anOther) const;
    uint64 GetHash() const;
  };
//---------------------------------------------------------------------------//
} }

