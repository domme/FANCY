#include "fancy_core_precompile.h"
#include "VertexInputLayoutDesc.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  bool VertexInputLayoutDesc::operator==(const VertexInputLayoutDesc& anOther) const
  {
    if (myAttributes.Size() != anOther.myAttributes.Size())
      return false;

    if (myBufferBindings.Size() != anOther.myBufferBindings.Size())
      return false;

    for (uint i = 0u; i < myBufferBindings.Size(); ++i)
      if (!(myBufferBindings[i] == anOther.myBufferBindings[i]))
        return false;

    for (uint i = 0u; i < myAttributes.Size(); ++i)
      if (!(myAttributes[i] == anOther.myAttributes[i]))
        return false;

    return true;
  }
//---------------------------------------------------------------------------//
  uint64 VertexInputLayoutDesc::GetHash() const
  {
    if (myAttributes.IsEmpty() && myBufferBindings.IsEmpty())
      return 0ull;

    MathUtil::BeginMultiHash();
    
    if (!myAttributes.IsEmpty())
      MathUtil::AddToMultiHash(myAttributes.GetBuffer(), myAttributes.ByteSize());
    if (!myBufferBindings.IsEmpty())
      MathUtil::AddToMultiHash(myBufferBindings.GetBuffer(), myBufferBindings.ByteSize());

    return MathUtil::EndMultiHash();
  }
//---------------------------------------------------------------------------//
}
