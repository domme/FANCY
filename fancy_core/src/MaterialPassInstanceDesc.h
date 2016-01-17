#pragma once

#include "FancyCorePrerequisites.h"
#include "Material.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct MaterialPassInstanceDesc
  {
    

    MaterialPassInstanceDesc() : myVertexAndIndexHash(0u) {}
    uint64 GetHash() const { return myVertexAndIndexHash; }
    bool operator==(const MaterialPassInstanceDesc& anOther) const { return myVertexAndIndexHash == anOther.myVertexAndIndexHash; }
  };
//---------------------------------------------------------------------------//
} }

#pragma once
