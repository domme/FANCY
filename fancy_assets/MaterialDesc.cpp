#include "MaterialDesc.h"

#include <fancy_core/MathUtil.h>

namespace Fancy {
//---------------------------------------------------------------------------//
  uint64 MaterialDesc::GetHash() const
  {
    uint64 hash = 0u;
    for (const String& tex : mySemanticTextures)
      MathUtil::hash_combine(hash, tex);

    for (const String& tex : myExtraTextures)
      MathUtil::hash_combine(hash, tex);

    for (float param : mySemanticParameters)
      MathUtil::hash_combine(hash, param);

    for (float param : myExtraParameters)
      MathUtil::hash_combine(hash, param);

    return hash;
  }
//---------------------------------------------------------------------------//
  bool MaterialDesc::operator==(const MaterialDesc& anOther) const
  {
    return GetHash() == anOther.GetHash();
  }
//---------------------------------------------------------------------------//
}


