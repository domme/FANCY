#include "VertexLayout.h"

namespace Fancy { namespace Core { namespace Rendering {
//---------------------------------------------------------------------------//
  VertexElement::VertexElement() :
    eSemantics(VertexSemantics::NONE),
    u32OffsetBytes(0u),
    u32SizeBytes(0u)
  {

  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  VertexLayout::VertexLayout() :
    m_u32StrideBytes(0u)
  {

  }
//---------------------------------------------------------------------------//
  VertexLayout::~VertexLayout()
  {

  }
//---------------------------------------------------------------------------//
  void VertexLayout::addVertexElement( const VertexElement& clVertexElement )
  {
    ASSERT_M(clVertexElement.u32SizeBytes > 0, "Invalid vertex element");

    // Adjacency-check. Currently we require all vertex elements to be specified in order to
    // avoid a sorting-step afterwards
    ASSERT_M((m_vVertexElements.size() == 0 && clVertexElement.u32OffsetBytes == 0) ||
              (m_vVertexElements.size() > 0 && m_vVertexElements.back().u32OffsetBytes
                + m_vVertexElements.back().u32SizeBytes == clVertexElement.u32OffsetBytes), 
                "Vertex-elements must be added in order");

    m_vVertexElements.push_back(clVertexElement);
    m_u32StrideBytes += clVertexElement.u32SizeBytes;
  }
//---------------------------------------------------------------------------//
} } }  // end of namespace Fancy::Core::Rendering