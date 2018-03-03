#pragma once

#include <fancy_core/FancyCorePrerequisites.h>
#include <fancy_core/TextureDesc.h>

namespace Fancy
{
  struct MaterialDesc
  {
    struct TexDesc
    {
      uint mySemantic;
      TextureDesc myTexture;
    };

    struct ParamDesc
    {
      uint mySemantic;
      float myValue;
    };
        
    DynamicArray<TexDesc> myTextures;
    DynamicArray<ParamDesc> myParams;
  };
}

