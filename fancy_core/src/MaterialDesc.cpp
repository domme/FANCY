#include "MaterialDesc.h"
#include "MathUtil.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  MaterialDesc::MaterialDesc()
  {
    memset(&myParameters[0], 0x0, sizeof(myParameters));
  }
//---------------------------------------------------------------------------//
  bool MaterialDesc::operator==(const MaterialDesc& anOther) const 
  {
    return GetHash() == anOther.GetHash();
  }
//---------------------------------------------------------------------------//
  uint64 MaterialDesc::GetHash() const
  {
    uint64 hash = 0u;
    MathUtil::hash_combine(hash, MathUtil::hashFromGeneric(myParameters));

    for (uint i = 0u; i < ARRAY_LENGTH(myPasses); ++i)
      MathUtil::hash_combine(hash, myPasses[i].GetHash());

    return hash;
  }
//---------------------------------------------------------------------------//
  void MaterialDesc::Serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serializeArray(myParameters, "myParameters");
    aSerializer->serializeArray(myPasses, "myPasses");
  }
//---------------------------------------------------------------------------//
  bool MaterialDesc::IsEmpty() const
  {
    for (uint i = 0u; i < ARRAY_LENGTH(myPasses); ++i)
      if (!myPasses[i].IsEmpty())
        return false;

    return true;
  }
//---------------------------------------------------------------------------//
} }