#include "Mesh.h"
#include "GeometryData.h"

namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  Mesh::Mesh() : m_Name(ObjectName::blank)
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
  void Mesh::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(m_Name, "m_Name");
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Geometry
