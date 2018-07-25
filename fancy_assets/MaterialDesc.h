#pragma once

#include <fancy_core/FancyCorePrerequisites.h>
#include "MaterialSemantics.h"

namespace Fancy
{
  struct MaterialDesc
  {
    uint64 GetHash() const;
    bool operator==(const MaterialDesc& anOther) const;

    FixedArray<String, (uint)TextureSemantic::NUM> mySemanticTextures;
    DynamicArray<String> myExtraTextures;

    FixedArray<float, (uint)ParameterSemantic::NUM> mySemanticParameters;
    DynamicArray<float> myExtraParameters;
  };
}

