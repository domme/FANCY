#pragma once

#include "EASTL/fixed_vector.h"
#include "FancyCoreDefines.h"
#include "RenderEnums.h"

namespace Fancy
{
  struct RaytracingShaderIdentifier
  {
    RaytracingShaderIdentifierType myType;
    eastl::fixed_vector<uint8, 64> myData;
  };
}
