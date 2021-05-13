#pragma once

#include "DX12Prerequisites.h"
#include "RootSignatureDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
  class RootSignatureCacheDX12
  {
  public:
    struct RootSignatureCacheEntry
    {
      Microsoft::WRL::ComPtr<ID3D12RootSignature> myRootSignature;
      SharedPtr<RootSignatureLayoutDX12> myLayout;
    };

    RootSignatureCacheDX12(const RenderPlatformProperties& someProperties);
    ~RootSignatureCacheDX12() = default;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> GetBindlessDefaultRootSignature() const { return myBindlessDefaultRootSignature.myRootSignature; }
    SharedPtr<RootSignatureLayoutDX12> GetBindlessDefaultRootSignatureLayout() const { return myBindlessDefaultRootSignature.myLayout; }

    bool ReplaceWithCached(SharedPtr<RootSignatureLayoutDX12>& aLayout, Microsoft::WRL::ComPtr<ID3D12RootSignature>& aRootSignature);

  private:
    void CreateBindlessRootSig(const RenderPlatformProperties& someProperties);

    RootSignatureCacheEntry myBindlessDefaultRootSignature;
    eastl::hash_map<uint64, RootSignatureCacheEntry> myCache;
  };
}

#endif