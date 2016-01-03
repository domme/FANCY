#include "VertexInputLayout.h"
#include "Serializer.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  namespace Validation 
  {
    void validateLayout(ShaderVertexInputElementList& vInputElements);
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  void Validation::validateLayout(ShaderVertexInputElementList& vInputElements)
  {
#if defined (FANCY_RENDERSYSTEM_USE_VALIDATION)
    for (uint32 i = 0u; i < vInputElements.size(); ++i)
    {
      const ShaderVertexInputElement& element = vInputElements[i];
      for (uint32 k = 0u; k < vInputElements.size(); ++k)
      {
        if (i == k) continue;

        const ShaderVertexInputElement& otherElement = vInputElements[k];
        ASSERT_M(element.myRegisterIndex != otherElement.myRegisterIndex,
          "Invalid vertex layout");
      }
    }
#endif  // FANCY_RENDERSYSTEM_USE_VALIDATION
  }
//---------------------------------------------------------------------------//
  void ShaderVertexInputLayout::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&myVertexInputElements, "VertexInputElements");
  }
  //---------------------------------------------------------------------------//
  ShaderVertexInputLayout::ShaderVertexInputLayout()
  {

  }
//---------------------------------------------------------------------------//
  ShaderVertexInputLayout::~ShaderVertexInputLayout()
  {

  }
//---------------------------------------------------------------------------//
  void ShaderVertexInputLayout::addVertexInputElement( const ShaderVertexInputElement& clVertexElement )
  {
    myVertexInputElements.push_back(clVertexElement);
    Validation::validateLayout(myVertexInputElements);
  }
//---------------------------------------------------------------------------//
  void ShaderVertexInputElement::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&myName, "myName");
    aSerializer->serialize(&mySemantics, "eSemantics");
    aSerializer->serialize(&myRegisterIndex, "registerIndex");
    aSerializer->serialize(&mySizeBytes, "sizeBytes");
    aSerializer->serialize(&myFormat, "dataFormat");
    aSerializer->serialize(&myFormatComponentCount, "formatComponentCount");
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering