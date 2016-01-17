#include "MaterialDesc.h"
#include "MathUtil.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  MaterialDesc::MaterialDesc()
  {
    // TODO: these can be changed to c-arrays again (serialization should support this by now...)

    myPasses.resize(myPasses.capacity());
    memset(&myPasses[0], 0u, sizeof(MaterialPassInstanceDesc) * myPasses.capacity());

    myParameters.resize(myParameters.capacity());
    memset(&myParameters[0], 0x0, sizeof(float) * myParameters.capacity());
  }
//---------------------------------------------------------------------------//
  bool MaterialDesc::operator==(const MaterialDesc& anOther) const 
  {
    return GetHash() == anOther.GetHash();
  }
//---------------------------------------------------------------------------//
  uint64 MaterialDesc::GetHash() const
  {
    float parameters_raw[(uint32)EMaterialParameterSemantic::NUM];

    for (uint i = 0u; i < myParameters.size(); ++i)
      parameters_raw[i] = myParameters[i];

    uint64 hash = 0u;
    MathUtil::hash_combine(hash, MathUtil::hashFromGeneric(parameters_raw));

    for (uint i = 0u; i < myPasses.size(); ++i)
      MathUtil::hash_combine(hash, myPasses[i].GetHash());

    return hash;
  }
//---------------------------------------------------------------------------//
} }