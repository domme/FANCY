#pragma once
#include "ShaderPipeline.h"
#include "DX12Prerequisites.h"

namespace Fancy
{
  class ShaderPipelineDX12 : public ShaderPipeline
  {
  public:

    void UpdateResourceInterface() override;

    ID3D12RootSignature* GetRootSignature() const { return myRootSignature.Get(); }

  protected:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> myRootSignature;

  };
}


