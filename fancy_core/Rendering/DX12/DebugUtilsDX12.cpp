#include "fancy_core_precompile.h"

#include "DebugUtilsDX12.h"
#include "ResourceBarrierStatesDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
  eastl::string DebugUtilsDX12::ResourceStatesToString( D3D12_RESOURCE_STATES aStates ) {
    if ( aStates == D3D12_RESOURCE_STATE_COMMON )
      return "common";

    eastl::string str;
    if ( aStates & D3D12_RESOURCE_STATE_GENERIC_READ ) {
      str += "generic read|";
      aStates = aStates & ( ~D3D12_RESOURCE_STATE_GENERIC_READ );
    }

    if ( aStates & D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER )
      str += "vertex constant buffer|";
    if ( aStates & D3D12_RESOURCE_STATE_INDEX_BUFFER )
      str += "index buffer|";
    if ( aStates & D3D12_RESOURCE_STATE_RENDER_TARGET )
      str += "render target|";
    if ( aStates & D3D12_RESOURCE_STATE_UNORDERED_ACCESS )
      str += "unordered access|";
    if ( aStates & D3D12_RESOURCE_STATE_DEPTH_WRITE )
      str += "depth write|";
    if ( aStates & D3D12_RESOURCE_STATE_DEPTH_READ )
      str += "depth read|";
    if ( aStates & D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE )
      str += "non-px resource|";
    if ( aStates & D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE )
      str += "px resource|";
    if ( aStates & D3D12_RESOURCE_STATE_STREAM_OUT )
      str += "stream out|";
    if ( aStates & D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT )
      str += "indirect|";
    if ( aStates & D3D12_RESOURCE_STATE_COPY_DEST )
      str += "copy dst|";
    if ( aStates & D3D12_RESOURCE_STATE_COPY_SOURCE )
      str += "copy src|";
    if ( aStates & D3D12_RESOURCE_STATE_RESOLVE_DEST )
      str += "resolve dst|";
    if ( aStates & D3D12_RESOURCE_STATE_RESOLVE_SOURCE )
      str += "resolve src|";
    if ( aStates & D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE )
      str += "rt acceleration|";

    if ( !str.empty() && str[ str.size() - 1 ] == '|' )
      str.erase( str.size() - 1 );

    return str;
  }

  eastl::string DebugUtilsDX12::TextureStateToString( uint s ) {
    if ( s == GPU_TEXTURE_STATE_UNDEFINED )
      return "undefined";
    if ( s == GPU_TEXTURE_STATE_COMMON )
      return "common";
    
    eastl::string str;
    if ( s & GPU_TEXTURE_STATE_SHADER_READ_PIXEL )
      str += "shader_read_px|";
    if ( s & GPU_TEXTURE_STATE_SHADER_READ_NON_PIXEL )
      str += "shader_read_non_px|";
    if ( s & GPU_TEXTURE_STATE_SHADER_WRITE )
      str += "shader_write|";
    if ( s & GPU_TEXTURE_STATE_RENDER_TARGET )
      str += "render_target|";
    if ( s & GPU_TEXTURE_STATE_DEPTH_WRITE )
      str += "depth_write|";
    if ( s & GPU_TEXTURE_STATE_DEPTH_READ )
      str += "depth_read|";
    if ( s & GPU_TEXTURE_STATE_COPY_SOURCE )
      str += "copy_src|";
    if ( s & GPU_TEXTURE_STATE_COPY_DEST )
      str += "copy_dst|";
    if ( s & GPU_TEXTURE_STATE_PRESENT )
      str += "present|";
    if ( !str.empty() && str.back() == '|' )
      str.erase( str.size() - 1 );
    
    return str;
  }

  eastl::string DebugUtilsDX12::BufferStateToString( uint s ) {
    if ( s == GPU_BUFFER_STATE_UNDEFINED )
      return "undefined";
    
    eastl::string str;
    if ( s & GPU_BUFFER_STATE_VERTEX_INDEX )
      str += "vertex_index|";
    if ( s & GPU_BUFFER_STATE_CONSTANT_BUFFER )
      str += "constant_buffer|";
    if ( s & GPU_BUFFER_STATE_SHADER_READ_PIXEL )
      str += "shader_read_px|";
    if ( s & GPU_BUFFER_STATE_SHADER_READ_NON_PIXEL )
      str += "shader_read_non_px|";
    if ( s & GPU_BUFFER_STATE_SHADER_WRITE )
      str += "shader_write|";
    if ( s & GPU_BUFFER_STATE_INDIRECT_ARGUMENT )
      str += "indirect|";
    if ( s & GPU_BUFFER_STATE_COPY_SOURCE )
      str += "copy_src|";
    if ( s & GPU_BUFFER_STATE_COPY_DEST )
      str += "copy_dst|";
    if ( s & GPU_BUFFER_STATE_RT_ACCELERATION_STRUCTURE )
      str += "rt_as|";
    if ( s & GPU_BUFFER_STATE_RT_AS_BUILD_SCRATCH )
      str += "rt_as_scratch|";
    if ( s & GPU_BUFFER_STATE_RT_SBT )
      str += "rt_sbt|";
    if ( !str.empty() && str.back() == '|' )
      str.erase( str.size() - 1 );
    
    return str;
  }
}

#endif