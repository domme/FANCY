#pragma once

#include "FancyCoreDefines.h"
#include "RenderEnums.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct TextureSamplerProperties
  {
    SamplerFilterMode   myMinFiltering = SamplerFilterMode::NEAREST;
    SamplerFilterMode   myMagFiltering = SamplerFilterMode::NEAREST;
    SamplerAddressMode  myAddressModeX = SamplerAddressMode::CLAMP_BORDER;
    SamplerAddressMode  myAddressModeY = SamplerAddressMode::CLAMP_BORDER;
    SamplerAddressMode  myAddressModeZ = SamplerAddressMode::CLAMP_BORDER;
    CompFunc            myComparisonFunc = CompFunc::NEVER;
    SamplerBorderColor  myBorderColor = SamplerBorderColor::FLOAT_TRANSPARENT_BLACK;
    float               myMinLod = 0.0f;
    float               myMaxLod = FLT_MAX;
    float               myLodBias = 0.0f;
    uint                myMaxAnisotropy = 1000;
  };
//---------------------------------------------------------------------------//
}
