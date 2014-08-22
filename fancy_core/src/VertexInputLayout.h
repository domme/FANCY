#ifndef INCLUDE_VERTEXINPUTLAYOUT_H
#define INCLUDE_VERTEXINPUTLAYOUT_H

#include "RendererPrerequisites.h"
#include "ObjectName.h"
#include "FixedArray.h"

namespace Fancy { namespace Core { namespace Rendering {
//---------------------------------------------------------------------------//
  struct VertexInputElement 
  {
    VertexInputElement() : 
      eSemantics(VertexSemantics::NONE), u32RegisterIndex(0u), 
      u32SizeBytes(0u), eFormat(DataFormat::NONE) {}

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
  };
//---------------------------------------------------------------------------//
  const uint32 kMaxNumInputVertexAttributes = 32;
  typedef FixedArray<VertexInputElement, kMaxNumInputVertexAttributes> VertexInputElementList;
//---------------------------------------------------------------------------//
  class VertexInputLayout
  {
  public:
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
} } }  // end of namespace Fancy::Core::Rendering


#endif  // INCLUDE_VERTEXLAYOUT_H