#pragma once

#include "DX12Prerequisites.h"

namespace Fancy
{
  struct ShaderResourceInfoDX12
  {
    uint64 myNameHash;  // The name of the resource in the shader source
    D3D12_ROOT_PARAMETER myRootParameter;  // The root parameter optained from shader reflection describing how to bind it to the shader
  };
}



