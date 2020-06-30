#pragma once

#include "FancyCoreDefines.h"
#include "MathIncludes.h"
#include "Ptr.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class TextureView;
//---------------------------------------------------------------------------//
  enum class MaterialTextureType
  {
    BASE_COLOR = 0,
    NORMAL,
    MATERIAL,

    NUM,
    NONE = ~0
  };
//---------------------------------------------------------------------------//
  enum class MaterialParameterType
  {
    DIFFUSE_REFLECTIVITY = 0,
    SPECULAR_REFLECTIVITY,
    SPECULAR_POWER,
    OPACITY,

    NUM,
    NONE = ~0
  };
//---------------------------------------------------------------------------//
  struct MaterialDesc
  {
    uint64 GetHash() const;

    String myTextures[(uint)MaterialTextureType::NUM];
    glm::float4 myParameters[(uint)MaterialParameterType::NUM] = {};
  };
//---------------------------------------------------------------------------//
  struct Material
  {
    MaterialDesc GetDescription() const;

    SharedPtr<TextureView> myTextures[(uint) MaterialTextureType::NUM];
    glm::float4 myParameters[(uint)MaterialParameterType::NUM] = {};
  };
//---------------------------------------------------------------------------//
}
