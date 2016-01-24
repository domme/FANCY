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
    FixedArray<float, (uint32)EMaterialParameterSemantic::NUM> myParameters;
    FixedArray<MaterialPassInstanceDesc, (uint32)EMaterialPass::NUM> myPasses;

    MaterialDesc();
    bool operator==(const MaterialDesc& anOther) const;
    uint64 GetHash() const;
  };
//---------------------------------------------------------------------------//
} }

