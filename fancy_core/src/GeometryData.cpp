
#include "GeometryData.h"

namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  GeometryData::GeometryData() :
    m_pIndexBuffer(nullptr),
    m_pVertexBuffer(nullptr)
  {

  }
//---------------------------------------------------------------------------//
  GeometryData::~GeometryData()
  {
    FANCY_DELETE(m_pVertexBuffer, MemoryCategory::GEOMETRY);
    FANCY_DELETE(m_pIndexBuffer, MemoryCategory::GEOMETRY);
  }
//---------------------------------------------------------------------------//
  GeometryDataDesc GeometryData::getDescription() const
  {
    GeometryDataDesc desc;
    desc.myName = m_name;
    desc.myVertexLayout = m_vertexLayout;
    desc.myVertexData = nullptr;
    desc.myIndexData = nullptr;

    if (m_pVertexBuffer)
    {
      desc.myVertexBufferParams = m_pVertexBuffer->getParameters();
    }

    if (m_pIndexBuffer)
    {
      desc.myIndexBufferParams = m_pIndexBuffer->getParameters();
    }

    return desc;
  }
//---------------------------------------------------------------------------//
  void GeometryData::initFromDescription(const GeometryDataDesc& aDesc)
  {
    ASSERT(aDesc.myVertexData);
    ASSERT(aDesc.myIndexData);

    FANCY_DELETE(m_pVertexBuffer, MemoryCategory::GEOMETRY);
    FANCY_DELETE(m_pIndexBuffer, MemoryCategory::GEOMETRY);

    m_name = aDesc.myName;
    m_vertexLayout = aDesc.myVertexLayout;
    
    m_pVertexBuffer = FANCY_NEW(Rendering::GpuBuffer, MemoryCategory::GEOMETRY);
    m_pIndexBuffer = FANCY_NEW(Rendering::GpuBuffer, MemoryCategory::GEOMETRY);

    m_pVertexBuffer->create(aDesc.myVertexBufferParams, aDesc.myVertexData);
    m_pIndexBuffer->create(aDesc.myIndexBufferParams, aDesc.myIndexData);
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering

