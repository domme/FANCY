#pragma once

#include "Common/FancyCoreDefines.h"
#include "Common/MathIncludes.h"
#include "Common/Ptr.h"

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
    COLOR = 0,
    SPECULAR_REFLECTIVITY,
    SPECULAR_POWER,
    OPACITY,
    EMISSION,

    NUM,
    NONE = ~0
  };
//---------------------------------------------------------------------------//
  struct MaterialDesc
  {
    uint64 GetHash() const;

    eastl::string myTextures[(uint)MaterialTextureType::NUM];
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
