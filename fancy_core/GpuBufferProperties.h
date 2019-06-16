#pragma once

#include "FancyCoreDefines.h"
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
      , myUsage(GpuBufferUsage::CONSTANT_BUFFER)
      , myDefaultState(GpuResourceUsageState::COMMON)
      , myIsShaderWritable(false)
    { }

    // TODO: Change this to only work with byte size instead of elements+elementSize
    uint64 myNumElements;
    uint64 myElementSizeBytes;
    CpuMemoryAccessType myCpuAccess;
    GpuBufferUsage myUsage;
    GpuResourceUsageState myDefaultState;
    bool myIsShaderWritable;
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
      , mySize(UINT64_MAX)
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
