#pragma once

#include <fancy_core/FancyCorePrerequisites.h>
#include "MaterialSemantics.h"
#include "MaterialDesc.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class TextureView;
//---------------------------------------------------------------------------//
  struct Material
  {
    Material();
    ~Material();
    MaterialDesc GetDescription() const;

    FixedArray<SharedPtr<TextureView>, (uint)TextureSemantic::NUM> mySemanticTextures;
    DynamicArray<SharedPtr<TextureView>> myExtraTextures;

    FixedArray<float, (uint)ParameterSemantic::NUM> mySemanticParameters;
    DynamicArray<float> myExtraParameters;
  };
//---------------------------------------------------------------------------//
}
