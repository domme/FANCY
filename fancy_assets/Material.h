#pragma once

#include <fancy_core/FancyCorePrerequisites.h>
#include "MaterialSemantics.h"
#include "MaterialDesc.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class Texture;
//---------------------------------------------------------------------------//
  struct Material
  {
    Material();
    MaterialDesc GetDescription() const;

    FixedArray<SharedPtr<Texture>, (uint)TextureSemantic::NUM> mySemanticTextures;
    DynamicArray<SharedPtr<Texture>> myExtraTextures;

    FixedArray<float, (uint)ParameterSemantic::NUM> mySemanticParameters;
    DynamicArray<float> myExtraParameters;
  };
//---------------------------------------------------------------------------//
}
