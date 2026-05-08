#pragma once

#include "Common/FancyCoreDefines.h"
#include "DX12Prerequisites.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
  namespace DebugUtilsDX12 {
    eastl::string BarrierLayoutToString( D3D12_BARRIER_LAYOUT aLayout );
    eastl::string BarrierAccessToString( D3D12_BARRIER_ACCESS anAccess );
    eastl::string BarrierSyncToString( D3D12_BARRIER_SYNC aSync );
  }
}  // namespace Fancy

#endif