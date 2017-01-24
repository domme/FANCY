#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "MathUtil.h"
#include "Serializer.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct TextureSamplerDesc 
  {
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
    float              fMaxAnisotropy;

    TextureSamplerDesc() :
      minFiltering(SamplerFilterMode::NEAREST),
      magFiltering(SamplerFilterMode::NEAREST),
      addressModeX(SamplerAddressMode::WRAP),
      addressModeY(SamplerAddressMode::WRAP),
      addressModeZ(SamplerAddressMode::WRAP),
      comparisonFunc(CompFunc::ALWAYS),
      borderColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)),
      fMinLod(0.0f),
      fMaxLod(FLT_MAX),
      fLodBias(0.0f),
      fMaxAnisotropy(1.0f) {}

    uint64 GetHash() const
    {
      return MathUtil::hashFromGeneric(*this);
    }

    bool operator==(const TextureSamplerDesc& anOther) const
    {
      return GetHash() == anOther.GetHash();
    }

    void Serialize(IO::Serializer* aSerializer)
    {
      aSerializer->Serialize(&minFiltering, "minFiltering");
      aSerializer->Serialize(&magFiltering, "magFiltering");
      aSerializer->Serialize(&addressModeX, "addressModeX");
      aSerializer->Serialize(&addressModeY, "addressModeY");
      aSerializer->Serialize(&addressModeZ, "addressModeZ");
      aSerializer->Serialize(&borderColor, "borderColor");
      aSerializer->Serialize(&fMinLod, "fMinLod");
      aSerializer->Serialize(&fMaxLod, "fMaxLod");
      aSerializer->Serialize(&fLodBias, "fLodBias");
      aSerializer->Serialize(&fMaxAnisotropy, "fMaxAnisotropy");
    }
  };
//---------------------------------------------------------------------------//
} }
