#ifndef INCLUDE_VERTEXLAYOUT_H
#define INCLUDE_VERTEXLAYOUT_H

#include "RendererPrerequisites.h"
#include "ObjectName.h"
#include "FixedArray.h"

namespace Fancy { namespace Core { namespace Rendering {
//---------------------------------------------------------------------------//
  struct GeometryVertexElement 
  {
    GeometryVertexElement();

    /// Name of the vertex attribute as imported from the mesh
    ObjectName name;
    /// Semantic of the attribute as imported from the mesh
    VertexSemantics eSemantics;
    /// Offset in bytes to the first occurance in the vertexBuffer
    uint32 u32OffsetBytes;
    /// Size of the attribute in bytes
    uint32 u32SizeBytes;
    /// The format of the element
    DataFormat eFormat;
  };
//---------------------------------------------------------------------------//
  const uint32 kMaxNumGeometryVertexAttributes = 32;
//---------------------------------------------------------------------------//
  typedef FixedArray<GeometryVertexElement, kMaxNumGeometryVertexAttributes> VertexElementList;
//---------------------------------------------------------------------------//
  class GeometryVertexLayout
  {
    public:
      GeometryVertexLayout();
      ~GeometryVertexLayout();

      void addVertexElement(const GeometryVertexElement& clVertexElement);
      const GeometryVertexElement& getVertexElement(uint32 u32Index) const { ASSERT(u32Index < m_vVertexElements.size()); return m_vVertexElements[u32Index]; }
      uint32 getNumVertexElements() const { return m_vVertexElements.size(); }
      uint32 getStrideBytes() const { return m_u32StrideBytes; }

      const VertexElementList& getVertexElementList() const { return m_vVertexElements; }

    private:
      uint32 m_u32StrideBytes;
      VertexElementList m_vVertexElements;
  };
//---------------------------------------------------------------------------//
} } }  // end of namespace Fancy::Core::Rendering


#endif  // INCLUDE_VERTEXLAYOUT_H