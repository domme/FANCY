#pragma once

#include "ShaderResourceInterface.h"
#include "DX12Prerequisites.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class ShaderResourceInterfaceDX12 : public ShaderResourceInterface
  {
  public:
    ~ShaderResourceInterfaceDX12() override;
    
    bool Create(const D3D12_ROOT_SIGNATURE_DESC& anRSDesc, ComPtr<ID3D12RootSignature> aRootSignature = nullptr);

    static ShaderResourceInterfaceDesc CreateDescription(const D3D12_ROOT_SIGNATURE_DESC& anRSdesc);
    static uint64 ComputeHash(const D3D12_ROOT_SIGNATURE_DESC& anRSdesc);

    ComPtr<ID3D12RootSignature> myRootSignature;
  };
//---------------------------------------------------------------------------//
} } }