#ifndef INCLUDE_GEOMETRYDATA_H
#define INCLUDE_GEOMETRYDATA_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "OpenGLprerequisites.h"
#include "VertexLayout.h"

namespace Fancy { namespace Core { namespace Geometry {

  class GeometryData {
    public:
      GeometryData();
      ~GeometryData();

      Rendering::GeometryVertexLayout* getVertexDeclaration() {return m_pVertexLayout;}
      Rendering::GpuBuffer* getVertexBuffer() {return m_pVertexBuffer;}
      Rendering::GpuBuffer* getIndexBuffer() {return m_pIndexBuffer;}

    protected:
      Rendering::GeometryVertexLayout* m_pVertexLayout;
      Rendering::GpuBuffer* m_pVertexBuffer;  // TODO: put into array for multiple vertex-streams?
      Rendering::GpuBuffer* m_pIndexBuffer;
  };

} } }  // end of namespace Fancy::Core::Geometry::

#endif  // INCLUDE_GEOMETRYDATA_H