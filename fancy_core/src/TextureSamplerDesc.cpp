#include "TextureSamplerDesc.h"
#include "Serializer.h"
#include "MathUtil.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  TextureSamplerDesc::TextureSamplerDesc():
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
    myMaxAnisotropy(1u)
  {
  }
//---------------------------------------------------------------------------//
  uint64 TextureSamplerDesc::GetHash() const
  {
    return MathUtil::ByteHash(*this);
  }
//---------------------------------------------------------------------------//
  bool TextureSamplerDesc::operator==(const TextureSamplerDesc& anOther) const
  {
    return GetHash() == anOther.GetHash();
  }
//---------------------------------------------------------------------------//
  void TextureSamplerDesc::Serialize(IO::Serializer* aSerializer)
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
    aSerializer->Serialize(&myMaxAnisotropy, "myMaxAnisotropy");
  }
//---------------------------------------------------------------------------//
} }
