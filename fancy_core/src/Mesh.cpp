#include "Mesh.h"
#include "GeometryData.h"

namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  Mesh::Mesh()
  {

  }
//---------------------------------------------------------------------------//
  Mesh::~Mesh()
  {

  }
//---------------------------------------------------------------------------//
  MeshDesc Mesh::getDescription() const
  {
    MeshDesc desc;
    desc.myName = m_Name;
    desc.myGeometryDatas.reserve(m_vGeometries.size());
    for (uint32 i = 0u; i < m_vGeometries.size(); ++i)
    {
      desc.myGeometryDatas.push_back(m_vGeometries[i]->getDescription());
    }

    return desc;
  }
//---------------------------------------------------------------------------//
  void Mesh::initFromDescription(const MeshDesc& aDesc)
  {

  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Geometry
