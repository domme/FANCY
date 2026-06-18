#pragma once

#include "Common/FancyCoreDefines.h"
#include "DX12Prerequisites.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
  namespace DebugUtilsDX12 {
    // Legacy function - kept for backward compatibility but not used by enhanced barriers
    eastl::string ResourceStatesToString( D3D12_RESOURCE_STATES aStates );
    
    // New enhanced barrier state formatting functions
    eastl::string TextureStateToString( uint aGpuTextureState );
    eastl::string BufferStateToString( uint aGpuBufferState );
  }
}  // namespace Fancy

#endif