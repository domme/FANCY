#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "DescriptionBase.h"

namespace Fancy { namespace IO {
  class Serializer;
} }

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct TextureSamplerDesc : public DescriptionBase
  {
    TextureSamplerDesc();
    ~TextureSamplerDesc() override = default;
    bool operator==(const TextureSamplerDesc& anOther) const;

    uint64 GetHash() const override;
    void Serialize(IO::Serializer* aSerializer) override;
    ObjectName GetTypeName() const override { return _N(TextureSampler); }

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
} }
