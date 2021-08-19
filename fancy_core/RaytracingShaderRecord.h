#pragma once

#include "EASTL/fixed_vector.h"
#include "FancyCoreDefines.h"
#include "RenderEnums.h"

namespace Fancy
{
  struct RaytracingShaderRecord
  {
    RaytracingShaderRecordType myType;
    eastl::fixed_vector<uint8, 64> myData;
  };
}
