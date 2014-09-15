#ifndef INCLUDE_GEOMETRYDATA_H
#define INCLUDE_GEOMETRYDATA_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "OpenGLprerequisites.h"
#include "GeometryVertexLayout.h"
#include "GpuBuffer.h"

namespace Fancy { namespace Core { namespace Geometry {

  class GeometryData {
    public:
      GeometryData();
      ~GeometryData();

      Rendering::GeometryVertexLayout* getGeometryVertexLayout() {return m_pVertexLayout;}
      Rendering::GpuBuffer* getVertexBuffer() {return m_pVertexBuffer;}
      Rendering::GpuBuffer* getIndexBuffer() {return m_pIndexBuffer;}
      uint32 getNumVertices() const {return m_pVertexBuffer ?  m_pVertexBuffer->getNumElements() : 0u; }
      uint32 getNumIndices() const {return m_pIndexBuffer ? m_pIndexBuffer->getNumElements() : 0u; }
      
    protected:
      Rendering::GeometryVertexLayout* m_pVertexLayout;
      Rendering::GpuBuffer* m_pVertexBuffer;  // TODO: put into array for multiple vertex-streams?
      Rendering::GpuBuffer* m_pIndexBuffer;
  };

} } }  // end of namespace Fancy::Core::Geometry::

#endif  // INCLUDE_GEOMETRYDATA_H