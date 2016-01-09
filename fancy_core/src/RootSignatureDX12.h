#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

#if defined(RENDERER_DX12)

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class RootSignatureDX12
  {
  public:
    ComPtr<ID3D12RootSignature> myRootSignature;
    uint myHash;
  };
//---------------------------------------------------------------------------//
  class RootSignaturePoolDX12
  {
    public:
      static RootSignatureDX12* CreateOrRetrieve(const D3D12_ROOT_SIGNATURE_DESC& anRSdesc, const ComPtr<ID3D12Device>& aDevice, 
        const ComPtr<ID3D12RootSignature>* anRS = nullptr);
      static void Init();
      static void Destroy();

    protected:
      static std::vector<RootSignatureDX12*> myRootSignaturePool;
  };
//---------------------------------------------------------------------------//
} } }

#endif  // RENDERER_DX12

