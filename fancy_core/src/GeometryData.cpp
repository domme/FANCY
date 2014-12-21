
#include "GeometryData.h"

namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  GeometryData::GeometryData() :
    m_pIndexBuffer(nullptr),
    m_pVertexBuffer(nullptr),
    m_uNumVertices(0u),
    m_uVertexStrideBytes(0u)
  {

  }
//---------------------------------------------------------------------------//
  GeometryData::~GeometryData()
  {
    FANCY_DELETE(m_pVertexBuffer, MemoryCategory::GEOMETRY);
    FANCY_DELETE(m_pIndexBuffer, MemoryCategory::GEOMETRY);
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering

