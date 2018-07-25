#pragma once

#include "FancyCorePrerequisites.h"
#include "MathUtil.h"

namespace Fancy {
//---------------------------------------------------------------------------//  
  struct GpuBufferDesc
  {
    uint myInternalRefIndex;  // The internal index of a texture used by the engine. Can be some semantics-enum defined by the renderingprocess (e.g. GBUFFER)

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
  };
//---------------------------------------------------------------------------//
  struct GpuBufferViewProperties
  {
    GpuBufferViewProperties()
      : myFormat(DataFormat::UNKNOWN)
      , myStructureSize(0u)
      , myIsConstantBuffer(false)
      , myIsShaderWritable(false)
      , myIsStructured(false)
      , myIsRaw(false)
      , myOffset(0u)
      , mySize(~0u)
    {}

    DataFormat myFormat;
    uint myStructureSize;
    bool myIsConstantBuffer;
    bool myIsShaderWritable;
    bool myIsStructured;
    bool myIsRaw;
    uint64 myOffset;
    uint64 mySize;
  };
//---------------------------------------------------------------------------//
}
