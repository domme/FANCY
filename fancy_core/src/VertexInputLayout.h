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
  struct VertexInputElement 
  {
    SERIALIZABLE(VertexInputElement)
    static ObjectName getTypeName() { return _N(VertexInputElement); }
    const ObjectName& getName() const { return ObjectName::blank; }
    void serialize(IO::Serializer* aSerializer);

    VertexInputElement() : 
      eSemantics(VertexSemantics::NONE), u32RegisterIndex(0u), 
      u32SizeBytes(0u), eFormat(DataFormat::NONE), uFormatComponentCount(1u) {}

    /// Name of the vertex attribute as reported by shader-reflection
    ObjectName name;
    /// Semantic of the attribute as reported by shader-reflection
    VertexSemantics eSemantics;
    /// Register-index in the shader
    uint32 u32RegisterIndex;
    /// Size of the attribute in bytes
    uint32 u32SizeBytes;
    /// The format of the element
    DataFormat eFormat;
    /// Multiplier for eFormat. Used for multi-component elements (e.g. Matrices)
    uint8 uFormatComponentCount;
  };
//---------------------------------------------------------------------------//
  const uint32 kMaxNumInputVertexAttributes = 16;
  typedef FixedArray<VertexInputElement, kMaxNumInputVertexAttributes> VertexInputElementList;
//---------------------------------------------------------------------------//
  class VertexInputLayout
  {
  public:
    SERIALIZABLE(VertexInputLayout)
    static ObjectName getTypeName() { return _N(VertexInputLayout); }
    const ObjectName& getName() const { return ObjectName::blank; }
    void serialize(IO::Serializer* aSerializer);

    VertexInputLayout();
    ~VertexInputLayout();

    void clear() {m_vVertexInputElements.clear();}
    void addVertexInputElement(const VertexInputElement& clVertexElement);
    const VertexInputElement& getVertexInputElement(uint32 u32Index) const { ASSERT(u32Index < m_vVertexInputElements.size()); return m_vVertexInputElements[u32Index]; }
    uint32 getNumVertexInputElements() const { return m_vVertexInputElements.size(); }
    const VertexInputElementList& getVertexElementList() const { return m_vVertexInputElements; }

  private:
    VertexInputElementList m_vVertexInputElements;
  };
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering


#endif  // INCLUDE_VERTEXLAYOUT_H