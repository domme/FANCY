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
    MathUtil::hash_combine(hash, MathUtil::Hash(mySourcePath));
    MathUtil::hash_combine(hash, myInternalRefIndex);
    return hash;
  }
//---------------------------------------------------------------------------//
  void TextureDesc::Serialize(IO::Serializer* aSerializer)
  {
    aSerializer->Serialize(&myIsExternalTexture, "myIsExternalTexture");
    aSerializer->Serialize(&mySourcePath, "mySourcePath");
    aSerializer->Serialize(&myInternalRefIndex, "myInternalRefIndex");
  }
//---------------------------------------------------------------------------//
} }
 