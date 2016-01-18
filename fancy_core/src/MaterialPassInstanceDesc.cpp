#include "FancyCorePrerequisites.h"
#include "MaterialPassInstanceDesc.h"

#include "MaterialPassInstanceDesc.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  uint64 MaterialPassInstanceDesc::GetHash() const
  {
    uint64 hash;
    MathUtil::hash_combine(hash, myMaterialPass.GetHash());
    
    for (uint i = 0u; i < Constants::kMaxNumReadTextures; ++i)
      MathUtil::hash_combine(hash, myReadTextures[i].GetHash());

    return hash;
  }
//---------------------------------------------------------------------------//
  bool MaterialPassInstanceDesc::operator==(const MaterialPassInstanceDesc& anOther) const
  {
  }

//---------------------------------------------------------------------------//
} }
