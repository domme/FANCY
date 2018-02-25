#pragma once

#include "TextureDesc.h"
#include "MathUtil.h"

namespace Fancy {
//---------------------------------------------------------------------------//  
  uint64 TextureDesc::GetHash() const
  {
    uint64 hash;
    MathUtil::hash_combine(hash, myIsExternalTexture);
    MathUtil::hash_combine(hash, MathUtil::Hash(mySourcePath));
    MathUtil::hash_combine(hash, myInternalRefIndex);
    return hash;
  }
//---------------------------------------------------------------------------//
}
 