#pragma once

#include "RendererPrerequisites.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct GeometryVertexElement 
  {
    GeometryVertexElement();
    
    /// Name of the vertex attribute as imported from the mesh
    String name;
    /// Semantic of the attribute as imported from the mesh
    VertexSemantics eSemantics;
    /// Index of the semantic for multi-semantic types
    uint mySemanticIndex;
    /// Offset in bytes to the first occurance in the vertexBuffer
    uint u32OffsetBytes;
    /// Size of the attribute in bytes
    uint u32SizeBytes;
    /// The format of the element
    DataFormat eFormat;
  };
//---------------------------------------------------------------------------//
  class GeometryVertexLayout
  {
    public:
      GeometryVertexLayout();
      ~GeometryVertexLayout();

      void addVertexElement(const GeometryVertexElement& clVertexElement);
      
      uint myStride;
      DynamicArray<GeometryVertexElement> m_vVertexElements;
  };
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering