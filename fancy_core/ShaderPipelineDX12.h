#pragma once
#include "ShaderPipeline.h"
#include "DX12Prerequisites.h"
#include "ShaderResourceInfoDX12.h"
#include "RootSignatureDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
  class ShaderPipelineDX12 : public ShaderPipeline
  {
  public:
    void CreateFromShaders() override;

    ID3D12RootSignature* GetRootSignature() const { return myRootSignature.Get(); }
    const RootSignatureLayoutDX12& GetRootSignatureLayout() const { return myRootSignatureLayout; }
    DynamicArray<ShaderResourceInfoDX12> GetResourceInfos() const { return myResourceInfos; }

  protected:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> myRootSignature;
    DynamicArray<ShaderResourceInfoDX12> myResourceInfos;
    RootSignatureLayoutDX12 myRootSignatureLayout;
  };
}

#endif