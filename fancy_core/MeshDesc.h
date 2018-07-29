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
    { }

    uint64 GetHash() const
    {
      uint64 hash = 0u;
      MathUtil::hash_combine(hash, myIsExternalMesh);
      MathUtil::hash_combine(hash, myUniqueName);
      return hash;
    }
    
    bool myIsExternalMesh;
    String myUniqueName;
  };
//---------------------------------------------------------------------------//
}
