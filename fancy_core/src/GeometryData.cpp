
#include "GeometryData.h"

namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  GeometryData::GeometryData() :
    m_pIndexBuffer(nullptr),
    m_pVertexBuffer(nullptr),
    m_pVertexLayout(nullptr),
    m_uNumVertices(0u),
    m_uVertexStrideBytes(0u)
  {

  }
//---------------------------------------------------------------------------//
  GeometryData::~GeometryData()
  {

  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering

