#pragma once

#include "RendererPrerequisites.h"

namespace Fancy {
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
      void AddVertexElement(VertexSemantics aSemantic, DataFormat aFormat, uint aSemanticIndex = 0u, const char* aName = "");
      
      TopologyType myTopology;
      uint myStride;
      DynamicArray<GeometryVertexElement> myElements;
  };
//---------------------------------------------------------------------------//
}