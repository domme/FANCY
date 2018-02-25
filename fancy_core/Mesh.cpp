#include "Mesh.h"
#include "GeometryData.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  Mesh::Mesh() : myVertexAndIndexHash(0u)
  {

  }
//---------------------------------------------------------------------------//
  Mesh::~Mesh()
  {
    for (uint i = 0u; i < m_vGeometries.size(); ++i)
    {
      FANCY_DELETE(m_vGeometries[i], MemoryCategory::Geometry);
    }
  }
//---------------------------------------------------------------------------//
  MeshDesc Mesh::GetDescription() const
  {
    MeshDesc desc;
    desc.myVertexAndIndexHash = myVertexAndIndexHash;
    return desc;
  }
//---------------------------------------------------------------------------//
  bool Mesh::operator==(const Mesh& anOther) const 
  {
    return myVertexAndIndexHash == anOther.myVertexAndIndexHash;
  }
//---------------------------------------------------------------------------//
  bool Mesh::operator==(const MeshDesc& aDesc) const 
  {
    return myVertexAndIndexHash == aDesc.myVertexAndIndexHash;
  }
//---------------------------------------------------------------------------//
}