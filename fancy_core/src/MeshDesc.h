#pragma once

#include "FancyCorePrerequisites.h"
#include "GeometryVertexLayout.h"

namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
// Because of the high amount of data involved in meshes, we'll completely rely on the BinaryCache to store the actual geometric
// data for meshes. The VertexAndIndexHash is just used to identify the requested mesh for Serialization
  struct MeshDesc
  {
    MeshDesc() : myVertexAndIndexHash(0u) {}
    bool operator==(const MeshDesc& anOther) const { return myVertexAndIndexHash == anOther.myVertexAndIndexHash; }

    uint64 myVertexAndIndexHash;
    DynamicArray<Rendering::GeometryVertexLayout> myVertexLayouts;
  };
//---------------------------------------------------------------------------//
} }
