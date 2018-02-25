#pragma once

#include "RendererPrerequisites.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct ShaderVertexInputElement 
  {
    uint64 GetHash() const;

    ShaderVertexInputElement() : 
      mySemantics(VertexSemantics::NONE), myRegisterIndex(0u), mySemanticIndex(0u),
      mySizeBytes(0u), myFormat(DataFormat::NONE), myFormatComponentCount(1u) {}

    /// Name of the vertex attribute as reported by shader-reflection
    String myName;
    /// Semantic of the attribute as reported by shader-reflection
    VertexSemantics mySemantics;
    /// Index of the sementic for multi-semantic types
    uint mySemanticIndex;
    /// Register-index in the shader
    uint myRegisterIndex;
    /// Size of the attribute in bytes
    uint mySizeBytes;
    /// The format of the element
    DataFormat myFormat;
    /// Multiplier for eFormat. Used for multi-component elements (e.g. Matrices)
    uint8 myFormatComponentCount;
  };
//---------------------------------------------------------------------------//
  class ShaderVertexInputLayout
  {
  public:
    static ShaderVertexInputLayout ourDefaultModelLayout;
    uint64 GetHash() const;

    void addVertexInputElement(const ShaderVertexInputElement& clVertexElement);
    ShaderVertexInputElement& addVertexInputElement();

    DynamicArray<ShaderVertexInputElement> myVertexInputElements;
  };
//---------------------------------------------------------------------------//
}
