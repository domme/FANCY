#pragma once

#include "FancyCorePrerequisites.h"

namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  struct MeshDesc
  {
    // At this point, we only store the hash computed from the vertex- and index data
    // as a description. We can't use it to re-create meshes but it'll suffice as a comparison
    uint64 myVertexAndIndexHash;

    MeshDesc() : myVertexAndIndexHash(0u) {}
    bool operator==(const MeshDesc& anOther) const { return myVertexAndIndexHash == anOther.myVertexAndIndexHash; }
  };
//---------------------------------------------------------------------------//
} }