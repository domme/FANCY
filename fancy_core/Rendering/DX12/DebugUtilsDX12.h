#pragma once

#include "Common/FancyCoreDefines.h"
#include "DX12Prerequisites.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
  namespace DebugUtilsDX12
  {
    eastl::string ResourceStatesToString(D3D12_RESOURCE_STATES aStates);
  }
}

#endif