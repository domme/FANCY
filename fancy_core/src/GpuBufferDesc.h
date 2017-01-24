#pragma once

#include "FancyCorePrerequisites.h"
#include "MathUtil.h"
#include "Serializer.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//  
  struct GpuBufferDesc
  {
    uint32 myInternalRefIndex;  // The internal index of a texture used by the engine. Can be some semantics-enum defined by the renderingprocess (e.g. GBUFFER)

    GpuBufferDesc() : myInternalRefIndex(~0u) {}

    bool operator==(const GpuBufferDesc& anOther) const {
      return myInternalRefIndex == anOther.myInternalRefIndex;
    }

    uint64 GetHash() const
    {
      uint64 hash;
      MathUtil::hash_combine(hash, myInternalRefIndex);
      return hash;
    }

    void Serialize(IO::Serializer* aSerializer)
    {
      aSerializer->Serialize(&myInternalRefIndex, "myInternalRefIndex");
    }
  };
//---------------------------------------------------------------------------//
} }
