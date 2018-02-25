#pragma once

#include "ShaderResourceInterface.h"
#include "DX12Prerequisites.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class ShaderResourceInterfaceDX12 : public ShaderResourceInterface
  {
  public:
    ~ShaderResourceInterfaceDX12() override = default;
    
    bool Create(const D3D12_ROOT_SIGNATURE_DESC& anRSDesc, const Microsoft::WRL::ComPtr<ID3D12RootSignature>& aRootSignature = nullptr);

    static ShaderResourceInterfaceDesc CreateDescription(const D3D12_ROOT_SIGNATURE_DESC& anRSdesc);
    static uint64 ComputeHash(const D3D12_ROOT_SIGNATURE_DESC& anRSdesc);

    Microsoft::WRL::ComPtr<ID3D12RootSignature> myRootSignature;
  };
  //---------------------------------------------------------------------------//
}