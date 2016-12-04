#ifndef INCLUDE_VERTEXLAYOUT_H
#define INCLUDE_VERTEXLAYOUT_H

#include "RendererPrerequisites.h"
#include "ObjectName.h"
#include "FixedArray.h"

namespace Fancy {namespace IO {
class Serializer;
}
}

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct GeometryVertexElement 
  {
    GeometryVertexElement();
    void Serialize(IO::Serializer* aSerializer);
    
    /// Name of the vertex attribute as imported from the mesh
    ObjectName name;
    /// Semantic of the attribute as imported from the mesh
    VertexSemantics eSemantics;
    /// Index of the semantic for multi-semantic types
    uint32 mySemanticIndex;
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
      void Serialize(IO::Serializer* aSerializer);

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
} }  // end of namespace Fancy::Rendering


#endif  // INCLUDE_VERTEXLAYOUT_H