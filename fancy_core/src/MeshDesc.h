#pragma once

#include "FancyCorePrerequisites.h"
#include "GeometryVertexLayout.h"
#include "DescriptionBase.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  class Serializer;
//---------------------------------------------------------------------------//
} }

namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  struct MeshDesc : public DescriptionBase
  {
    // Because of the high amount of data involved in meshes, we'll completely rely on the BinaryCache to store the actual geometric
    // data for meshes. The VertexAndIndexHash is just used to identify the requested mesh for Serialization
    uint64 myVertexAndIndexHash;

    std::vector<Rendering::GeometryVertexLayout> myVertexLayouts;

    MeshDesc() : myVertexAndIndexHash(0u) {}
    ~MeshDesc() override {}

    ObjectName GetTypeName() const override { return _N(Mesh); }
    bool operator==(const MeshDesc& anOther) const { return myVertexAndIndexHash == anOther.myVertexAndIndexHash; }
    uint64 GetHash() const override { return myVertexAndIndexHash; }
    void Serialize(IO::Serializer* aSerializer) override;
    bool IsEmpty() const override { return myVertexAndIndexHash == 0u; }
  };
//---------------------------------------------------------------------------//
} }