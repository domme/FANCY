#pragma once

#include "FancyCorePrerequisites.h"
#include "MathUtil.h"

using namespace Fancy;

namespace Fancy {
//---------------------------------------------------------------------------//
  struct MeshDesc
  {
    MeshDesc() 
      : myIsExternalMesh(true)
      , myInternalRefIdx(~0u)
    { }

    uint64 GetHash() const
    {
      uint64 hash = 0u;
      MathUtil::hash_combine(hash, myIsExternalMesh);
      MathUtil::hash_combine(hash, myInternalRefIdx);
      MathUtil::hash_combine(hash, myUniqueName);
      return hash;
    }
    
    bool myIsExternalMesh;
    uint myInternalRefIdx;
    String myUniqueName;
  };
//---------------------------------------------------------------------------//
}
