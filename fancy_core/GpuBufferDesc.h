#pragma once

#include "FancyCorePrerequisites.h"
#include "MathUtil.h"

namespace Fancy {
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
