#pragma once

#include "MaterialSemantics.h"

#include <fancy_core/FancyCoreDefines.h>
#include <fancy_core/FixedArray.h>
#include <fancy_core/DynamicArray.h>

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
