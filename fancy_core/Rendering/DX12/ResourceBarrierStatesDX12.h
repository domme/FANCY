#pragma once

#if FANCY_ENABLE_DX12

#include "Common/FancyCoreDefines.h"
#include "Debug/Log.h"
#include "Rendering/RenderEnums.h"
#include <d3d12.h>
#include <EASTL/fixed_vector.h>

namespace Fancy {
  enum GpuBufferState : uint {
  GPU_BUFFER_STATE_UNDEFINED            = 0,
  GPU_BUFFER_STATE_VERTEX_INDEX         = 1u << 0u,
  GPU_BUFFER_STATE_CONSTANT_BUFFER      = 1u << 1u,
  GPU_BUFFER_STATE_SHADER_READ_PIXEL    = 1u << 2u,   // SRV from pixel shader
  GPU_BUFFER_STATE_SHADER_READ_NON_PIXEL = 1u << 3u,  // SRV from vertex/geometry/compute/RT shaders
  GPU_BUFFER_STATE_SHADER_WRITE         = 1u << 4u,   // UAV
  GPU_BUFFER_STATE_INDIRECT_ARGUMENT    = 1u << 5u,
  GPU_BUFFER_STATE_COPY_SOURCE          = 1u << 6u,
  GPU_BUFFER_STATE_COPY_DEST            = 1u << 7u,
  GPU_BUFFER_STATE_RT_ACCELERATION_STRUCTURE = 1u << 8u,  // Read as RTAS during raytracing
  GPU_BUFFER_STATE_RT_AS_BUILD_SCRATCH  = 1u << 9u,   // Scratch buffer for AS build
  GPU_BUFFER_STATE_RT_SBT               = 1u << 10u,  // Shader binding table
  // Derived helpers
  GPU_BUFFER_STATE_SHADER_READ_ALL = GPU_BUFFER_STATE_SHADER_READ_PIXEL | GPU_BUFFER_STATE_SHADER_READ_NON_PIXEL,
};

enum GpuTextureState : uint {
  GPU_TEXTURE_STATE_UNDEFINED            = 0,
  GPU_TEXTURE_STATE_SHADER_READ_PIXEL    = 1u << 0u,  // SRV from pixel shader
  GPU_TEXTURE_STATE_SHADER_READ_NON_PIXEL = 1u << 1u, // SRV from vertex/geometry/compute/RT
  GPU_TEXTURE_STATE_SHADER_WRITE         = 1u << 2u,  // UAV (unordered access)
  GPU_TEXTURE_STATE_RENDER_TARGET        = 1u << 3u,
  GPU_TEXTURE_STATE_DEPTH_WRITE          = 1u << 4u,
  GPU_TEXTURE_STATE_DEPTH_READ           = 1u << 5u,
  GPU_TEXTURE_STATE_COPY_SOURCE          = 1u << 6u,
  GPU_TEXTURE_STATE_COPY_DEST            = 1u << 7u,
  GPU_TEXTURE_STATE_PRESENT              = 1u << 8u,
  GPU_TEXTURE_STATE_COMMON               = 1u << 9u,  // D3D12_BARRIER_LAYOUT_COMMON
  // Derived helper
  GPU_TEXTURE_STATE_SHADER_READ_ALL = GPU_TEXTURE_STATE_SHADER_READ_PIXEL | GPU_TEXTURE_STATE_SHADER_READ_NON_PIXEL,
};

inline bool IsBufferWriteState( uint aState ) {
  return ( aState & ( GPU_BUFFER_STATE_SHADER_WRITE |
                      GPU_BUFFER_STATE_COPY_DEST |
                      GPU_BUFFER_STATE_RT_AS_BUILD_SCRATCH ) ) != 0u;
}

inline bool IsTextureWriteState( uint aState ) {
  return ( aState & ( GPU_TEXTURE_STATE_SHADER_WRITE |
                      GPU_TEXTURE_STATE_RENDER_TARGET |
                      GPU_TEXTURE_STATE_DEPTH_WRITE |
                      GPU_TEXTURE_STATE_COPY_DEST ) ) != 0u;
}

struct GpuSubresourceBarrierStateDX12 {
  GpuSubresourceBarrierStateDX12()
    : myState( GPU_TEXTURE_STATE_UNDEFINED ), myContext( CommandListType::Graphics ) {}

  union {
    uint                  myState;
    D3D12_RESOURCE_STATES myStates;
  };
  CommandListType myContext;
};

struct GpuResourceBarrierDataDX12 {
  GpuResourceBarrierDataDX12()
    : myCanChangeStates( true ), myAllSubresourcesSameState( true ), myReadStates( 0u ), myWriteStates( 0u ) {}

  bool myCanChangeStates; // TODO: Double-check if this is still needed after transitioning to enhanced barriers. If not, remove it and simplify the code that currently checks it.
  bool myAllSubresourcesSameState;
  uint myReadStates;
  uint myWriteStates;
  eastl::fixed_vector< GpuSubresourceBarrierStateDX12, 16 > mySubresources;
};

using GpuSubresourceHazardDataDX12 = GpuSubresourceBarrierStateDX12;
using GpuResourceHazardDataDX12 = GpuResourceBarrierDataDX12;

struct TextureBarrierParams {
  D3D12_BARRIER_SYNC   mySync;
  D3D12_BARRIER_ACCESS myAccess;
  D3D12_BARRIER_LAYOUT myLayout;
};

inline TextureBarrierParams GetTextureBarrierParams( uint aState ) {
  TextureBarrierParams p = { D3D12_BARRIER_SYNC_NONE, D3D12_BARRIER_ACCESS_NO_ACCESS,
                             D3D12_BARRIER_LAYOUT_UNDEFINED };

  if ( aState == GPU_TEXTURE_STATE_UNDEFINED ) return p;  // stays all-none/undefined

  if ( aState == GPU_TEXTURE_STATE_COMMON ) {
    p.mySync   = D3D12_BARRIER_SYNC_ALL;
    p.myAccess = D3D12_BARRIER_ACCESS_COMMON;
    p.myLayout = D3D12_BARRIER_LAYOUT_COMMON;
    return p;
  }

  // Write states — each is mutually exclusive, check in order:
  if ( aState & GPU_TEXTURE_STATE_RENDER_TARGET ) {
    p.mySync   = D3D12_BARRIER_SYNC_RENDER_TARGET;
    p.myAccess = D3D12_BARRIER_ACCESS_RENDER_TARGET;
    p.myLayout = D3D12_BARRIER_LAYOUT_RENDER_TARGET;
    return p;
  }
  if ( aState & GPU_TEXTURE_STATE_DEPTH_WRITE ) {
    p.mySync   = D3D12_BARRIER_SYNC_DEPTH_STENCIL;
    p.myAccess = D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;
    p.myLayout = D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
    return p;
  }
  if ( aState & GPU_TEXTURE_STATE_COPY_DEST ) {
    p.mySync   = D3D12_BARRIER_SYNC_COPY;
    p.myAccess = D3D12_BARRIER_ACCESS_COPY_DEST;
    p.myLayout = D3D12_BARRIER_LAYOUT_COPY_DEST;
    return p;
  }
  if ( aState & GPU_TEXTURE_STATE_SHADER_WRITE ) {
    p.mySync   = D3D12_BARRIER_SYNC_ALL_SHADING;
    p.myAccess = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
    p.myLayout = D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
    return p;
  }
  if ( aState & GPU_TEXTURE_STATE_PRESENT ) {
    // PRESENT = resource is scanned out by display hardware; no GPU access
    p.mySync   = D3D12_BARRIER_SYNC_NONE;
    p.myAccess = D3D12_BARRIER_ACCESS_NO_ACCESS;
    p.myLayout = D3D12_BARRIER_LAYOUT_PRESENT;
    return p;
  }
  // Depth read (before shader read to avoid misclassifying combined states):
  if ( aState & GPU_TEXTURE_STATE_DEPTH_READ ) {
    p.mySync   = D3D12_BARRIER_SYNC_DEPTH_STENCIL;
    p.myAccess = D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;
    p.myLayout = D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
    return p;
  }
  if ( aState & GPU_TEXTURE_STATE_COPY_SOURCE ) {
    p.mySync   = D3D12_BARRIER_SYNC_COPY;
    p.myAccess = D3D12_BARRIER_ACCESS_COPY_SOURCE;
    p.myLayout = D3D12_BARRIER_LAYOUT_COPY_SOURCE;
    return p;
  }
  // SHADER_READ_PIXEL and/or SHADER_READ_NON_PIXEL — both use the same layout:
  if ( aState & GPU_TEXTURE_STATE_SHADER_READ_ALL ) {
    p.myAccess = D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
    p.myLayout = D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
    const bool pixelOnly = ( aState & GPU_TEXTURE_STATE_SHADER_READ_ALL ) == GPU_TEXTURE_STATE_SHADER_READ_PIXEL;
    p.mySync = pixelOnly ? D3D12_BARRIER_SYNC_PIXEL_SHADING : D3D12_BARRIER_SYNC_ALL_SHADING;
    return p;
  }

  ASSERT( false, "Unhandled GpuTextureState" );
  return p;
}

struct BufferBarrierParams {
  D3D12_BARRIER_SYNC   mySync;
  D3D12_BARRIER_ACCESS myAccess;
};

inline BufferBarrierParams GetBufferBarrierParams( uint aState ) {
  BufferBarrierParams p = { D3D12_BARRIER_SYNC_NONE, D3D12_BARRIER_ACCESS_COMMON };

  if ( aState == GPU_BUFFER_STATE_UNDEFINED )
    return p;  // Already set to NONE + COMMON

  // Write states — mutually exclusive, check first:
  if ( aState & GPU_BUFFER_STATE_SHADER_WRITE ) {
    p.mySync   = D3D12_BARRIER_SYNC_ALL_SHADING;
    p.myAccess = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
    return p;
  }
  if ( aState & GPU_BUFFER_STATE_COPY_DEST ) {
    p.mySync   = D3D12_BARRIER_SYNC_COPY;
    p.myAccess = D3D12_BARRIER_ACCESS_COPY_DEST;
    return p;
  }
  if ( aState & GPU_BUFFER_STATE_RT_AS_BUILD_SCRATCH ) {
    // Scratch buffer has D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, not RTAS flag
    p.mySync   = D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE;
    p.myAccess = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
    return p;
  }

  // Read states — combineable; accumulate sync and access:
  if ( aState & GPU_BUFFER_STATE_VERTEX_INDEX ) {
    p.mySync   |= D3D12_BARRIER_SYNC_VERTEX_SHADING | D3D12_BARRIER_SYNC_INDEX_INPUT;
    p.myAccess |= D3D12_BARRIER_ACCESS_VERTEX_BUFFER | D3D12_BARRIER_ACCESS_INDEX_BUFFER;
  }
  if ( aState & GPU_BUFFER_STATE_CONSTANT_BUFFER ) {
    p.mySync   |= D3D12_BARRIER_SYNC_ALL_SHADING;
    p.myAccess |= D3D12_BARRIER_ACCESS_CONSTANT_BUFFER;
  }
  if ( aState & GPU_BUFFER_STATE_SHADER_READ_PIXEL ) {
    p.mySync   |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
    p.myAccess |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
  }
  if ( aState & GPU_BUFFER_STATE_SHADER_READ_NON_PIXEL ) {
    p.mySync   |= D3D12_BARRIER_SYNC_ALL_SHADING;
    p.myAccess |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
  }
  if ( aState & GPU_BUFFER_STATE_INDIRECT_ARGUMENT ) {
    p.mySync   |= D3D12_BARRIER_SYNC_EXECUTE_INDIRECT;
    p.myAccess |= D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT;
  }
  if ( aState & GPU_BUFFER_STATE_COPY_SOURCE ) {
    p.mySync   |= D3D12_BARRIER_SYNC_COPY;
    p.myAccess |= D3D12_BARRIER_ACCESS_COPY_SOURCE;
  }
  if ( aState & GPU_BUFFER_STATE_RT_ACCELERATION_STRUCTURE ) {
    p.mySync   |= D3D12_BARRIER_SYNC_RAYTRACING;
    p.myAccess |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ;
  }
  if ( aState & GPU_BUFFER_STATE_RT_SBT ) {
    p.mySync   |= D3D12_BARRIER_SYNC_RAYTRACING;
    p.myAccess |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
  }
  return p;
}

// =============================================================================
// Enhanced Barrier Layout Conversion Helpers
// =============================================================================
// Convert new state enums to D3D12_BARRIER_LAYOUT for CreatePlacedResource2()

inline D3D12_BARRIER_LAYOUT GetTextureLayoutFromState( uint aInitialState ) {
  // Map our texture states to D3D12_BARRIER_LAYOUT for CreatePlacedResource2
  if ( aInitialState & GPU_TEXTURE_STATE_RENDER_TARGET )
    return D3D12_BARRIER_LAYOUT_RENDER_TARGET;
  if ( aInitialState & GPU_TEXTURE_STATE_DEPTH_WRITE )
    return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
  if ( aInitialState & GPU_TEXTURE_STATE_DEPTH_READ )
    return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
  if ( aInitialState & GPU_TEXTURE_STATE_SHADER_WRITE )
    return D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
  if ( aInitialState & GPU_TEXTURE_STATE_COPY_DEST )
    return D3D12_BARRIER_LAYOUT_COPY_DEST;
  if ( aInitialState & GPU_TEXTURE_STATE_COPY_SOURCE )
    return D3D12_BARRIER_LAYOUT_COPY_SOURCE;
  if ( aInitialState & GPU_TEXTURE_STATE_PRESENT )
    return D3D12_BARRIER_LAYOUT_PRESENT;

  // Default for shader readable
  return D3D12_BARRIER_LAYOUT_COMMON;
}

}  // namespace Fancy

#endif
