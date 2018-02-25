#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct TextureSamplerDesc
  {
    TextureSamplerDesc();
    bool operator==(const TextureSamplerDesc& anOther) const;

    uint64 GetHash() const;

    SamplerFilterMode  minFiltering;
    SamplerFilterMode  magFiltering;
    SamplerAddressMode addressModeX;
    SamplerAddressMode addressModeY;
    SamplerAddressMode addressModeZ;
    CompFunc           comparisonFunc;
    glm::vec4          borderColor;
    float              fMinLod;
    float              fMaxLod;
    float              fLodBias;
    uint               myMaxAnisotropy;
  };
//---------------------------------------------------------------------------//
}
