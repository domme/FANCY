#pragma once

#include "FancyCorePrerequisites.h"
#include "GeometryVertexLayout.h"
#include "GpuBuffer.h"

namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  class GeometryData {
    public:
      GeometryData();
      ~GeometryData();

      const Rendering::GeometryVertexLayout& getGeometryVertexLayout() const {return m_vertexLayout;}
      const Rendering::GpuBuffer* getVertexBuffer() const {return m_pVertexBuffer.get();}
      const Rendering::GpuBuffer* getIndexBuffer() const {return m_pIndexBuffer.get();}
      uint getNumVertices() const {return m_pVertexBuffer ? m_pVertexBuffer->GetNumElements() : 0u; }
      uint getNumIndices() const {return m_pIndexBuffer ? m_pIndexBuffer->GetNumElements() : 0u; }
      uint getVertexStrideBytes() const {return m_vertexLayout.getStrideBytes();}
      
      void setVertexBuffer(const SharedPtr<Rendering::GpuBuffer>& _pVertexBuffer) {m_pVertexBuffer = _pVertexBuffer;}
      void setIndexBuffer(const SharedPtr<Rendering::GpuBuffer>& _pIndexBuffer) {m_pIndexBuffer = _pIndexBuffer;}
      void setVertexLayout(const Rendering::GeometryVertexLayout& _rVertexLayout) {m_vertexLayout = _rVertexLayout;}

    protected:
      Rendering::GeometryVertexLayout m_vertexLayout;
      SharedPtr<Rendering::GpuBuffer> m_pVertexBuffer;  // TODO: put into array for multiple vertex-streams?
      SharedPtr<Rendering::GpuBuffer> m_pIndexBuffer;
  };
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Geometry::