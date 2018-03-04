#pragma once

namespace Fancy {
  enum class TextureSemantic
  {
    BASE_COLOR = 0,
    NORMAL,
    MATERIAL,

    NUM,
    NONE = ~0
  };

  enum class ParameterSemantic
  {
    DIFFUSE_REFLECTIVITY = 0,
    SPECULAR_REFLECTIVITY,
    SPECULAR_POWER,
    OPACITY,

    NUM,
    NONE = ~0
  };
}
