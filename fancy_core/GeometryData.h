#pragma once

#include "FancyCoreDefines.h"
#include "GeometryVertexLayout.h"
#include "GpuBuffer.h"
#include "Ptr.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class GeometryData {
    public:
      const GeometryVertexLayout& getGeometryVertexLayout() const {return m_vertexLayout;}
      const GpuBuffer* getVertexBuffer() const {return m_pVertexBuffer.get();}
      const GpuBuffer* getIndexBuffer() const {return m_pIndexBuffer.get();}
      uint64 getNumVertices() const {return m_pVertexBuffer ? m_pVertexBuffer->GetProperties().myNumElements : 0u; }
      uint64 getNumIndices() const {return m_pIndexBuffer ? m_pIndexBuffer->GetProperties().myNumElements : 0u; }
      uint getVertexStrideBytes() const {return m_vertexLayout.myStride;}
      
      void setVertexBuffer(SharedPtr<GpuBuffer>& _pVertexBuffer) {m_pVertexBuffer = _pVertexBuffer;}
      void setIndexBuffer(SharedPtr<GpuBuffer>& _pIndexBuffer) {m_pIndexBuffer = _pIndexBuffer;}
      void setVertexLayout(const GeometryVertexLayout& _rVertexLayout) {m_vertexLayout = _rVertexLayout;}

    protected:
      GeometryVertexLayout m_vertexLayout;
      SharedPtr<GpuBuffer> m_pVertexBuffer;  // TODO: put into array for multiple vertex-streams?
      SharedPtr<GpuBuffer> m_pIndexBuffer;
  };
//---------------------------------------------------------------------------//
}