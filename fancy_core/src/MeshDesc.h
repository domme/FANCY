#pragma once

#include "FancyCorePrerequisites.h"
#include "GeometryVertexLayout.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  class Serializer;
//---------------------------------------------------------------------------//
} }

namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  struct MeshDesc
  {
    // Because of the high amount of data involved in meshes, we'll completely rely on the BinaryCache to store the actual geometric
    // data for meshes. The VertexAndIndexHash is just used to identify the requested mesh for Serialization
    uint64 myVertexAndIndexHash;

    std::vector<Rendering::GeometryVertexLayout> myVertexLayouts;

    MeshDesc() : myVertexAndIndexHash(0u) {}
    bool operator==(const MeshDesc& anOther) const { return myVertexAndIndexHash == anOther.myVertexAndIndexHash; }
    uint64 GetHash() const { return myVertexAndIndexHash; }

    void Serialize(IO::Serializer* aSerializer);
  };
//---------------------------------------------------------------------------//
} }