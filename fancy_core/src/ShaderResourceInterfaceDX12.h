#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "ShaderResourceInterfaceDesc.h"

#if defined(RENDERER_DX12)

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class ShaderResourceInterfaceDX12
  {
  public:
    ComPtr<ID3D12RootSignature> myRootSignature;
    uint myHash;
    ShaderResourceInterfaceDesc myInterfaceDesc;
  };
//---------------------------------------------------------------------------//
} } }

#endif  // RENDERER_DX12

