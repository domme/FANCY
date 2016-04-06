#include "VertexInputLayout.h"
#include "Serializer.h"
#include "MathUtil.h"

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
        ASSERT(element.myRegisterIndex != otherElement.myRegisterIndex,
          "Invalid vertex layout");
      }
    }
#endif  // FANCY_RENDERSYSTEM_USE_VALIDATION
  }
//---------------------------------------------------------------------------//
  uint64 ShaderVertexInputElement::GetHash() const
  {
    uint64 hash = 0;
    MathUtil::hash_combine(hash, myName.getHash());
    MathUtil::hash_combine(hash, (uint32)mySemantics);
    MathUtil::hash_combine(hash, mySemanticIndex);
    MathUtil::hash_combine(hash, myRegisterIndex);
    MathUtil::hash_combine(hash, mySizeBytes);
    MathUtil::hash_combine(hash, (uint32)myFormat);
    MathUtil::hash_combine(hash, myFormatComponentCount);
    return hash;
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
  ShaderVertexInputLayout ShaderVertexInputLayout::ourDefaultModelLayout;
//---------------------------------------------------------------------------//
  uint64 ShaderVertexInputLayout::GetHash() const
  {
    uint64 hash = 0;
    
    for (uint32 i = 0u; i < myVertexInputElements.size(); ++i)
      MathUtil::hash_combine(hash, myVertexInputElements[i].GetHash());
    
    return hash;
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
  ShaderVertexInputElement& ShaderVertexInputLayout::addVertexInputElement()
  {
    ShaderVertexInputElement elem;
    myVertexInputElements.push_back(elem);

    return myVertexInputElements.back();
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering