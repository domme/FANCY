#pragma once

#if FANCY_ENABLE_DX12

#include "DX12Prerequisites.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct RootSignatureDX12
  {
    RootSignatureDX12(const RenderPlatformProperties& someProperties);
    ID3D12RootSignature* GetRootSignature() const { return myRootSignature.Get(); }
    ID3D12RootSignature* GetLocalRootSignature() const { return myLocalRootSignature.Get(); }

    Microsoft::WRL::ComPtr<ID3D12RootSignature> myRootSignature;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> myLocalRootSignature;

    uint myRootParamIndex_GlobalResources = 0;
    uint myRootParamIndex_GlobalSamplers = 0;
    uint myRootParamIndex_LocalBuffers = 0;
    uint myNumLocalBuffers = 0;
    uint myRootParamIndex_LocalRWBuffers = 0;
    uint myNumLocalRWBuffers = 0;
    uint myRootParamIndex_LocalCBuffers = 0;
    uint myNumLocalCBuffers = 0;

  private:
    void CreateGlobalRootSignature(const RenderPlatformProperties& someProperties);
    void CreateLocalRootSignature();
  };
//---------------------------------------------------------------------------//
}



#endif

