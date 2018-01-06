#ifndef INCLUDE_VERTEXINPUTLAYOUT_H
#define INCLUDE_VERTEXINPUTLAYOUT_H

#include "RendererPrerequisites.h"
#include "ObjectName.h"
#include "FixedArray.h"
#include "Serializable.h"

namespace Fancy { namespace IO {
  class Serializer;
} }

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct ShaderVertexInputElement 
  {
    SERIALIZABLE(ShaderVertexInputElement)
    static ObjectName getTypeName() { return _N(ShaderVertexInputElement); }
    uint64 GetHash() const;
    void Serialize(IO::Serializer* aSerializer);

    ShaderVertexInputElement() : 
      mySemantics(VertexSemantics::NONE), myRegisterIndex(0u), mySemanticIndex(0u),
      mySizeBytes(0u), myFormat(DataFormat::NONE), myFormatComponentCount(1u) {}

    /// Name of the vertex attribute as reported by shader-reflection
    ObjectName myName;
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
  const uint kMaxNumInputVertexAttributes = 16;
  typedef FixedArray<ShaderVertexInputElement, kMaxNumInputVertexAttributes> ShaderVertexInputElementList;
//---------------------------------------------------------------------------//
  class ShaderVertexInputLayout
  {
  public:
    SERIALIZABLE(ShaderVertexInputLayout)
    static ObjectName getTypeName() { return _N(ShaderVertexInputLayout); }
    uint64 GetHash() const;
    void Serialize(IO::Serializer* aSerializer);

    ShaderVertexInputLayout();
    ~ShaderVertexInputLayout();

    void clear() {myVertexInputElements.clear();}
    void addVertexInputElement(const ShaderVertexInputElement& clVertexElement);
    ShaderVertexInputElement& addVertexInputElement();
    const ShaderVertexInputElement& getVertexInputElement(uint u32Index) const { ASSERT(u32Index < myVertexInputElements.size()); return myVertexInputElements[u32Index]; }
    uint getNumVertexInputElements() const { return myVertexInputElements.size(); }
    const ShaderVertexInputElementList& getVertexElementList() const { return myVertexInputElements; }

    static ShaderVertexInputLayout ourDefaultModelLayout;

  private:
    ShaderVertexInputElementList myVertexInputElements;
  };
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering


#endif  // INCLUDE_VERTEXLAYOUT_H