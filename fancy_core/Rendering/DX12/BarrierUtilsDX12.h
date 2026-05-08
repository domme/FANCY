#pragma once

#include "DX12Prerequisites.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
  //---------------------------------------------------------------------------//
  namespace BarrierUtilsDX12 {
    //---------------------------------------------------------------------------//
    // Returns the appropriate queue-specific SHADER_RESOURCE layout for a texture used as SRV
    // on the given command list type. Using queue-specific layouts avoids unnecessary cross-queue
    // promotions when the resource is only ever accessed from one queue type.
    //---------------------------------------------------------------------------//
    inline D3D12_BARRIER_LAYOUT GetShaderResourceLayout( CommandListType aCommandListType ) {
      switch ( aCommandListType ) {
        case CommandListType::Graphics:
          return D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_SHADER_RESOURCE;
        case CommandListType::Compute:
          return D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_SHADER_RESOURCE;
        default:
          return D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
      }
    }

    //---------------------------------------------------------------------------//
    // Returns the appropriate queue-specific UNORDERED_ACCESS layout for a texture used as UAV.
    //---------------------------------------------------------------------------//
    inline D3D12_BARRIER_LAYOUT GetUAVLayout( CommandListType aCommandListType ) {
      switch ( aCommandListType ) {
        case CommandListType::Graphics:
          return D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_UNORDERED_ACCESS;
        case CommandListType::Compute:
          return D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_UNORDERED_ACCESS;
        default:
          return D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
      }
    }

    //---------------------------------------------------------------------------//
    // Returns the appropriate queue-specific COPY_SOURCE layout.
    //---------------------------------------------------------------------------//
    inline D3D12_BARRIER_LAYOUT GetCopySourceLayout( CommandListType aCommandListType ) {
      switch ( aCommandListType ) {
        case CommandListType::Graphics:
          return D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COPY_SOURCE;
        case CommandListType::Compute:
          return D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COPY_SOURCE;
        default:
          return D3D12_BARRIER_LAYOUT_COPY_SOURCE;
      }
    }

    //---------------------------------------------------------------------------//
    // Returns the appropriate queue-specific COPY_DEST layout.
    //---------------------------------------------------------------------------//
    inline D3D12_BARRIER_LAYOUT GetCopyDestLayout( CommandListType aCommandListType ) {
      switch ( aCommandListType ) {
        case CommandListType::Graphics:
          return D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COPY_DEST;
        case CommandListType::Compute:
          return D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COPY_DEST;
        default:
          return D3D12_BARRIER_LAYOUT_COPY_DEST;
      }
    }
  }  // namespace BarrierUtilsDX12
  //---------------------------------------------------------------------------//
}  // namespace Fancy

#endif
