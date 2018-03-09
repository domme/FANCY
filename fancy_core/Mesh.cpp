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
}
