#pragma once

#include <fancy_core/FancyCorePrerequisites.h>

namespace Fancy {
//---------------------------------------------------------------------------//
  class Texture;
//---------------------------------------------------------------------------//
  class Material
  {
  public:
    
    struct Tex
    {
      enum Semantic
      {
        BASE_COLOR = 0,
        NORMAL,
        MATERIAL,

        NUM,
        NONE = ~0
      };

      Semantic mySemantic;
      SharedPtr<Texture> myTexture;
    };
  
    struct Param
    {
      enum Semantic
      {
        DIFFUSE_REFLECTIVITY = 0,
        SPECULAR_REFLECTIVITY,
        SPECULAR_POWER,
        OPACITY,

        NUM
      };

      Semantic mySemantic;
      float myValue;
    };

    DynamicArray<Param> myParameters;
    DynamicArray<Tex> myTextures;
  };
//---------------------------------------------------------------------------//
}
