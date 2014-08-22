#include "VertexInputLayout.h"

namespace Fancy { namespace Core { namespace Rendering {
//---------------------------------------------------------------------------//
  namespace Validation 
  {
    void validateLayout(VertexInputElementList& vInputElements);
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  void Validation::validateLayout(VertexInputElementList& vInputElements)
  {
#if defined (FANCY_RENDERSYSTEM_USE_VALIDATION)
    for (uint32 i = 0u; i < vInputElements.size(); ++i)
    {
      const VertexInputElement& element = vInputElements[i];
      for (uint32 k = 0u; k < vInputElements.size(); ++k)
      {
        if (i == k) continue;

        const VertexInputElement& otherElement = vInputElements[k];
        ASSERT_M(element.u32RegisterIndex != otherElement.u32RegisterIndex,
          "Invalid vertex layout");
      }
    }
#endif  // FANCY_RENDERSYSTEM_USE_VALIDATION
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
    Validation::validateLayout(m_vVertexInputElements);
  }
//---------------------------------------------------------------------------//

} } }  // end of namespace Fancy::Core::Rendering