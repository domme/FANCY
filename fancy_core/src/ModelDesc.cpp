#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "ModelDesc.h"
#include "MathUtil.h"
#include "Serializer.h"

namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  bool ModelDesc::operator==(const ModelDesc& anOther) const 
  {
    return GetHash() == anOther.GetHash();
  }
//---------------------------------------------------------------------------//
  uint64 ModelDesc::GetHash() const 
  {
    uint64 hash = 0;

    for (const Geometry::SubModelDesc& smDesc : mySubmodels)
      MathUtil::hash_combine(hash, smDesc.GetHash());

    return hash;
  }
//---------------------------------------------------------------------------//
  void ModelDesc::Serialize(IO::Serializer* aSerializer)
  {
    aSerializer->Serialize(&mySubmodels, "mySubmodels");
  }
//---------------------------------------------------------------------------//
  bool ModelDesc::IsEmpty() const
  {
    for (const SubModelDesc& desc : mySubmodels)
      if (!desc.IsEmpty())
        return false;

    return true;
  }
//---------------------------------------------------------------------------//
} }

