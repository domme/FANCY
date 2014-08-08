#include "VertexInputLayout.h"

namespace Fancy { namespace Core { namespace Rendering {
//---------------------------------------------------------------------------//
  VertexInputElement::VertexInputElement() :
    eSemantics(VertexSemantics::NONE),
    u32RegisterIndex(0u),
    u32SizeBytes(0u)
  {

  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  VertexInputLayout::VertexInputLayout()
  {

  }
//---------------------------------------------------------------------------//
  VertexInputLayout::~VertexInputLayout()
  {

  }
//---------------------------------------------------------------------------//
  void VertexInputLayout::addVertexInputElement( const VertexInputElement& clVertexElement )
  {
    m_vVertexInputElements.push_back(clVertexElement);
  }
//---------------------------------------------------------------------------//

} } }  // end of namespace Fancy::Core::Rendering