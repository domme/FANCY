#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "ModelDesc.h"

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
} }

