#include "Mesh.h"
#include "GeometryData.h"

namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  Mesh::Mesh() : m_Name(ObjectName::blank), myVertexAndIndexMD5(0u)
  {

  }
//---------------------------------------------------------------------------//
  Mesh::~Mesh()
  {
    for (uint32 i = 0u; i < m_vGeometries.size(); ++i)
    {
      FANCY_DELETE(m_vGeometries[i], MemoryCategory::Geometry);
    }
  }
//---------------------------------------------------------------------------//
  MeshDesc Mesh::GetDescription()
  {
    MeshDesc desc;
    desc.myVertexAndIndexMD5 = myVertexAndIndexMD5;
    return desc;
  }
//---------------------------------------------------------------------------//
  bool Mesh::operator==(const Mesh& anOther) const 
  {
    return myVertexAndIndexMD5 == anOther.myVertexAndIndexMD5;
  }
//---------------------------------------------------------------------------//
  bool Mesh::operator==(const MeshDesc& aDesc) const 
  {
    return myVertexAndIndexMD5 == aDesc.myVertexAndIndexMD5;
  }
//---------------------------------------------------------------------------//
  void Mesh::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&m_Name, "m_Name");
    aSerializer->serialize(&myVertexAndIndexMD5, "VertexAndIndexHash");
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Geometry
