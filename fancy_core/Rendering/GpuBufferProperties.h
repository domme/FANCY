#pragma once

#include "Common/FancyCoreDefines.h"
#include "RenderEnums.h"
#include "DataFormat.h"

namespace Fancy {
//---------------------------------------------------------------------------//  
  struct GpuBufferProperties
  {
    GpuBufferProperties()
      : myNumElements(0u)
      , myElementSizeBytes(0u)
      , myCpuAccess(CpuMemoryAccessType::NO_CPU_ACCESS)
      , myBindFlags((uint)GpuBufferBindFlags::NONE)
      , myIsShaderWritable(false)
    { }

    // TODO: Change this to only work with byte size instead of elements+elementSize
    uint64 myNumElements;
    uint64 myElementSizeBytes;
    CpuMemoryAccessType myCpuAccess;
    uint myBindFlags;
    bool myIsShaderWritable;
  };
//---------------------------------------------------------------------------//  
  struct GpuBufferViewProperties
  {
    GpuBufferViewProperties()
      : myFormat(DataFormat::UNKNOWN)
      , myStructureSize(0u)
      , myIsConstantBuffer(false)  // TODO: Use GpuBufferViewType instead of these bools where only one can be true at a time
      , myIsShaderWritable(false)
      , myIsStructured(false)
      , myIsRaw(false)
      , myIsRtAccelerationStructure(false)
      , myOffset(0u)
      , mySize(UINT64_MAX)
    {}

    DataFormat myFormat;
    uint myStructureSize;
    bool myIsConstantBuffer;
    bool myIsShaderWritable;
    bool myIsStructured;
    bool myIsRaw;
    bool myIsRtAccelerationStructure;
    uint64 myOffset;
    uint64 mySize;
  };
//---------------------------------------------------------------------------//
}
