#pragma once

#include <fancy_core/FancyCorePrerequisites.h>
#include <fancy_core/TextureDesc.h>
#include "MaterialSemantics.h"

namespace Fancy
{
  struct MaterialDesc
  {
    uint64 GetHash() const;
    bool operator==(const MaterialDesc& anOther) const;

    FixedArray<TextureDesc, (uint)TextureSemantic::NUM> mySemanticTextures;
    DynamicArray<TextureDesc> myExtraTextures;

    FixedArray<float, (uint)ParameterSemantic::NUM> mySemanticParameters;
    DynamicArray<float> myExtraParameters;
  };
}

