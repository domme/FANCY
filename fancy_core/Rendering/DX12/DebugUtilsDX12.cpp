#include "fancy_core_precompile.h"

#include "DebugUtilsDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
  eastl::string DebugUtilsDX12::ResourceStatesToString(D3D12_RESOURCE_STATES aStates)
  {
    if (aStates == D3D12_RESOURCE_STATE_COMMON)
      return "common";

    eastl::string str;
    if (aStates & D3D12_RESOURCE_STATE_GENERIC_READ)
    {
      str += "generic read|";
      aStates = aStates & (~D3D12_RESOURCE_STATE_GENERIC_READ);
    }

    if (aStates & D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
      str += "vertex constant buffer|";
    if (aStates & D3D12_RESOURCE_STATE_INDEX_BUFFER)
      str += "index buffer|";
    if (aStates & D3D12_RESOURCE_STATE_RENDER_TARGET)
      str += "render target|";
    if (aStates & D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
      str += "unordered access|";
    if (aStates & D3D12_RESOURCE_STATE_DEPTH_WRITE)
      str += "depth write|";
    if (aStates & D3D12_RESOURCE_STATE_DEPTH_READ)
      str += "depth read|";
    if (aStates & D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
      str += "non-px resource|";
    if (aStates & D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
      str += "px resource|";
    if (aStates & D3D12_RESOURCE_STATE_STREAM_OUT)
      str += "stream out|";
    if (aStates & D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT)
      str += "indirect|";
    if (aStates & D3D12_RESOURCE_STATE_COPY_DEST)
      str += "copy dst|";
    if (aStates & D3D12_RESOURCE_STATE_COPY_SOURCE)
      str += "copy src|";
    if (aStates & D3D12_RESOURCE_STATE_RESOLVE_DEST)
      str += "resolve dst|";
    if (aStates & D3D12_RESOURCE_STATE_RESOLVE_SOURCE)
      str += "resolve src|";
    if (aStates & D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE)
      str += "rt acceleration|";

    if (!str.empty() && str[str.size() - 1] == '|')
      str.erase(str.size() - 1);

    return str;
  }
}

#endif