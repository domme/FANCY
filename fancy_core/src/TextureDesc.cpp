#pragma once

#include "TextureDesc.h"
#include "Serializer.h"
#include "MathUtil.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//  
  uint64 TextureDesc::GetHash() const
  {
    uint64 hash;
    MathUtil::hash_combine(hash, myIsExternalTexture);
    MathUtil::hash_combine(hash, MathUtil::hashFromString(mySourcePath));
    MathUtil::hash_combine(hash, myInternalRefIndex);
    return hash;
  }
//---------------------------------------------------------------------------//
  void TextureDesc::Serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&myIsExternalTexture, "myIsExternalTexture");
    aSerializer->serialize(&mySourcePath, "mySourcePath");
    aSerializer->serialize(&myInternalRefIndex, "myInternalRefIndex");
  }
//---------------------------------------------------------------------------//
} }
 