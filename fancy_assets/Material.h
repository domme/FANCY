#pragma once

#include <fancy_core/FancyCorePrerequisites.h>

namespace Fancy {
//---------------------------------------------------------------------------//
  class Texture;
//---------------------------------------------------------------------------//
  class Material
  {
  public:
    enum class ParameterSemantic
    {
      DIFFUSE_REFLECTIVITY = 0,
      SPECULAR_REFLECTIVITY,
      SPECULAR_POWER,
      OPACITY,

      NUM
    };
  
    enum class TextureSemantic
    {
      BASE_COLOR = 0,
      NORMAL,
      MATERIAL,

      NUM,
      NONE = ~0
    };

    struct MaterialTexture
    {
      TextureSemantic mySemantic;
      SharedPtr<Texture> myTexture;
    };
  
    struct MaterialParameter
    {
      ParameterSemantic mySemantic;
      float myValue;
    };

    DynamicArray<MaterialParameter> myParameters;
    DynamicArray<MaterialTexture> myTextures;
  };
//---------------------------------------------------------------------------//
}
