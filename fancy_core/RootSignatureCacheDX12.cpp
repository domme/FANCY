#include "fancy_core_precompile.h"
#include "RootSignatureCacheDX12.h"

namespace Fancy
{
  bool RootSignatureCacheDX12::ReplaceWithCached(SharedPtr<RootSignatureLayoutDX12>& aLayout, Microsoft::WRL::ComPtr<ID3D12RootSignature>& aRootSignature)
  {
    const uint64 hash = aLayout->GetHash();
    auto it = myCache.find(hash);

    if (it != myCache.end())
    {
      aLayout = it->second.myLayout;
      aRootSignature = it->second.myRootSignature;
      return true;
    }

    myCache[hash] = { aRootSignature, aLayout };
    return false;
  }
}

