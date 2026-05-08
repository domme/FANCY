#include "fancy_core_precompile.h"

#include "DebugUtilsDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
  eastl::string DebugUtilsDX12::BarrierLayoutToString( D3D12_BARRIER_LAYOUT aLayout ) {
    switch ( aLayout ) {
      case D3D12_BARRIER_LAYOUT_UNDEFINED:
        return "UNDEFINED";
      case D3D12_BARRIER_LAYOUT_COMMON:
        return "COMMON/PRESENT";
      case D3D12_BARRIER_LAYOUT_RENDER_TARGET:
        return "RENDER_TARGET";
      case D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS:
        return "UNORDERED_ACCESS";
      case D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE:
        return "DEPTH_STENCIL_WRITE";
      case D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ:
        return "DEPTH_STENCIL_READ";
      case D3D12_BARRIER_LAYOUT_SHADER_RESOURCE:
        return "SHADER_RESOURCE";
      case D3D12_BARRIER_LAYOUT_COPY_SOURCE:
        return "COPY_SOURCE";
      case D3D12_BARRIER_LAYOUT_COPY_DEST:
        return "COPY_DEST";
      case D3D12_BARRIER_LAYOUT_RESOLVE_SOURCE:
        return "RESOLVE_SOURCE";
      case D3D12_BARRIER_LAYOUT_RESOLVE_DEST:
        return "RESOLVE_DEST";
      case D3D12_BARRIER_LAYOUT_SHADING_RATE_SOURCE:
        return "SHADING_RATE_SOURCE";
      case D3D12_BARRIER_LAYOUT_VIDEO_DECODE_READ:
        return "VIDEO_DECODE_READ";
      case D3D12_BARRIER_LAYOUT_VIDEO_DECODE_WRITE:
        return "VIDEO_DECODE_WRITE";
      case D3D12_BARRIER_LAYOUT_VIDEO_PROCESS_READ:
        return "VIDEO_PROCESS_READ";
      case D3D12_BARRIER_LAYOUT_VIDEO_PROCESS_WRITE:
        return "VIDEO_PROCESS_WRITE";
      case D3D12_BARRIER_LAYOUT_VIDEO_ENCODE_READ:
        return "VIDEO_ENCODE_READ";
      case D3D12_BARRIER_LAYOUT_VIDEO_ENCODE_WRITE:
        return "VIDEO_ENCODE_WRITE";
      case D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COMMON:
        return "DIRECT_QUEUE_COMMON";
      case D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ:
        return "DIRECT_QUEUE_GENERIC_READ";
      case D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_UNORDERED_ACCESS:
        return "DIRECT_QUEUE_UNORDERED_ACCESS";
      case D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_SHADER_RESOURCE:
        return "DIRECT_QUEUE_SHADER_RESOURCE";
      case D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COPY_SOURCE:
        return "DIRECT_QUEUE_COPY_SOURCE";
      case D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COPY_DEST:
        return "DIRECT_QUEUE_COPY_DEST";
      case D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COMMON:
        return "COMPUTE_QUEUE_COMMON";
      case D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_GENERIC_READ:
        return "COMPUTE_QUEUE_GENERIC_READ";
      case D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_UNORDERED_ACCESS:
        return "COMPUTE_QUEUE_UNORDERED_ACCESS";
      case D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_SHADER_RESOURCE:
        return "COMPUTE_QUEUE_SHADER_RESOURCE";
      case D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COPY_SOURCE:
        return "COMPUTE_QUEUE_COPY_SOURCE";
      case D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COPY_DEST:
        return "COMPUTE_QUEUE_COPY_DEST";
      default: {
        eastl::string s = "LAYOUT(";
        s += eastl::to_string( ( int ) aLayout );
        s += ")";
        return s;
      }
    }
  }
  //---------------------------------------------------------------------------//
  eastl::string DebugUtilsDX12::BarrierAccessToString( D3D12_BARRIER_ACCESS anAccess ) {
    if ( anAccess == D3D12_BARRIER_ACCESS_NO_ACCESS )
      return "NO_ACCESS";
    if ( anAccess == D3D12_BARRIER_ACCESS_COMMON )
      return "COMMON";

    eastl::string str;
    auto          append = [ & ]( D3D12_BARRIER_ACCESS bit, const char * name ) {
      if ( anAccess & bit )
        str += eastl::string( name ) + "|";
    };
    append( D3D12_BARRIER_ACCESS_VERTEX_BUFFER, "VERTEX_BUFFER" );
    append( D3D12_BARRIER_ACCESS_CONSTANT_BUFFER, "CONSTANT_BUFFER" );
    append( D3D12_BARRIER_ACCESS_INDEX_BUFFER, "INDEX_BUFFER" );
    append( D3D12_BARRIER_ACCESS_RENDER_TARGET, "RENDER_TARGET" );
    append( D3D12_BARRIER_ACCESS_UNORDERED_ACCESS, "UNORDERED_ACCESS" );
    append( D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE, "DEPTH_STENCIL_WRITE" );
    append( D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ, "DEPTH_STENCIL_READ" );
    append( D3D12_BARRIER_ACCESS_SHADER_RESOURCE, "SHADER_RESOURCE" );
    append( D3D12_BARRIER_ACCESS_STREAM_OUTPUT, "STREAM_OUTPUT" );
    append( D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT, "INDIRECT_ARGUMENT" );
    append( D3D12_BARRIER_ACCESS_COPY_DEST, "COPY_DEST" );
    append( D3D12_BARRIER_ACCESS_COPY_SOURCE, "COPY_SOURCE" );
    append( D3D12_BARRIER_ACCESS_RESOLVE_DEST, "RESOLVE_DEST" );
    append( D3D12_BARRIER_ACCESS_RESOLVE_SOURCE, "RESOLVE_SOURCE" );
    append( D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ, "RTAS_READ" );
    append( D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE, "RTAS_WRITE" );
    append( D3D12_BARRIER_ACCESS_SHADING_RATE_SOURCE, "SHADING_RATE_SOURCE" );
    if ( !str.empty() && str.back() == '|' )
      str.pop_back();
    return str;
  }
  //---------------------------------------------------------------------------//
  eastl::string DebugUtilsDX12::BarrierSyncToString( D3D12_BARRIER_SYNC aSync ) {
    if ( aSync == D3D12_BARRIER_SYNC_NONE )
      return "NONE";
    if ( aSync == D3D12_BARRIER_SYNC_ALL )
      return "ALL";

    eastl::string str;
    auto          append = [ & ]( D3D12_BARRIER_SYNC bit, const char * name ) {
      if ( aSync & bit )
        str += eastl::string( name ) + "|";
    };
    append( D3D12_BARRIER_SYNC_DRAW, "DRAW" );
    append( D3D12_BARRIER_SYNC_INDEX_INPUT, "INDEX_INPUT" );
    append( D3D12_BARRIER_SYNC_VERTEX_SHADING, "VERTEX_SHADING" );
    append( D3D12_BARRIER_SYNC_PIXEL_SHADING, "PIXEL_SHADING" );
    append( D3D12_BARRIER_SYNC_DEPTH_STENCIL, "DEPTH_STENCIL" );
    append( D3D12_BARRIER_SYNC_RENDER_TARGET, "RENDER_TARGET" );
    append( D3D12_BARRIER_SYNC_COMPUTE_SHADING, "COMPUTE_SHADING" );
    append( D3D12_BARRIER_SYNC_RAYTRACING, "RAYTRACING" );
    append( D3D12_BARRIER_SYNC_COPY, "COPY" );
    append( D3D12_BARRIER_SYNC_RESOLVE, "RESOLVE" );
    append( D3D12_BARRIER_SYNC_EXECUTE_INDIRECT, "EXECUTE_INDIRECT" );
    append( D3D12_BARRIER_SYNC_ALL_SHADING, "ALL_SHADING" );
    append( D3D12_BARRIER_SYNC_NON_PIXEL_SHADING, "NON_PIXEL_SHADING" );
    append( D3D12_BARRIER_SYNC_EMIT_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO, "RTAS_POSTBUILD" );
    append( D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE, "BUILD_RTAS" );
    append( D3D12_BARRIER_SYNC_COPY_RAYTRACING_ACCELERATION_STRUCTURE, "COPY_RTAS" );
    if ( !str.empty() && str.back() == '|' )
      str.pop_back();
    return str;
  }
}  // namespace Fancy

#endif