#pragma once

#include "ShaderResourceInterface.h"
#include "DX12Prerequisites.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class ShaderResourceInterfaceDX12 : public ShaderResourceInterface
  {
  public:
    ~ShaderResourceInterfaceDX12() override;

    ComPtr<ID3D12RootSignature> myRootSignature;
  };
//---------------------------------------------------------------------------//
} } }

