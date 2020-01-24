#pragma once
#include "ShaderPipeline.h"
#include "DX12Prerequisites.h"
#include "ShaderResourceInfoDX12.h"

namespace Fancy
{
  class ShaderPipelineDX12 : public ShaderPipeline
  {
  public:
    void UpdateResourceInterface() override;

    ID3D12RootSignature* GetRootSignature() const { return myRootSignature.Get(); }
    DynamicArray<ShaderResourceInfoDX12> GetResourceInfos() const { return myResourceInfos; }

  protected:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> myRootSignature;
    DynamicArray<ShaderResourceInfoDX12> myResourceInfos;
  };
}


