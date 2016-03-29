#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

#if defined(RENDERER_DX12)

#include "RootSignatureDX12.h"
#include "MathUtil.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  uint locComputeRSdescHash(const D3D12_ROOT_SIGNATURE_DESC& anRSdesc)
  {
    uint hash = 0u;
    MathUtil::hash_combine(hash, anRSdesc.NumParameters);
    for (uint i = 0u; i < anRSdesc.NumParameters; ++i)
      MathUtil::hash_combine(hash, MathUtil::hashFromGeneric(anRSdesc.pParameters[i]));

    MathUtil::hash_combine(hash, anRSdesc.NumStaticSamplers);
    for (uint i = 0u; i < anRSdesc.NumStaticSamplers; ++i)
      MathUtil::hash_combine(hash, MathUtil::hashFromGeneric(anRSdesc.pStaticSamplers[i]));

    MathUtil::hash_combine(hash, anRSdesc.Flags);

    return hash;
  }
//---------------------------------------------------------------------------//
  std::vector<RootSignatureDX12*> RootSignaturePoolDX12::myRootSignaturePool;
//---------------------------------------------------------------------------//
  RootSignatureDX12* RootSignaturePoolDX12::CreateOrRetrieve(const D3D12_ROOT_SIGNATURE_DESC& anRSdesc, const ComPtr<ID3D12Device>& aDevice, ComPtr<ID3D12RootSignature> anRS /* = nullptr*/)
  {
   const uint& requestedHash = locComputeRSdescHash(anRSdesc);

    for (RootSignatureDX12* rs : myRootSignaturePool)
      if (rs->myHash == requestedHash)
        return rs;

    RootSignatureDX12* rs = FANCY_NEW(RootSignatureDX12, MemoryCategory::MATERIALS);
    myRootSignaturePool.push_back(rs);
    rs->myHash = requestedHash;
    
    if (anRS == nullptr)
    {
      ComPtr<ID3DBlob> signature;
      ComPtr<ID3DBlob> error;
      ComPtr<ID3D12RootSignature> rootSignature;
      HRESULT success = D3D12SerializeRootSignature(&anRSdesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
      ASSERT_M(success == S_OK, "Failed serializing RootSignature");

      success = aDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
      ASSERT_M(success == S_OK, "Failed creating RootSignature");

      rs->myRootSignature = rootSignature;
    }
    else
    {
      rs->myRootSignature = anRS;
    }

    return rs;
  }
//---------------------------------------------------------------------------//
  void RootSignaturePoolDX12::Init()
  {
  
  }
//---------------------------------------------------------------------------//
  void RootSignaturePoolDX12::Destroy()
  {
    for (RootSignatureDX12* rs : myRootSignaturePool)
      FANCY_DELETE(rs, MemoryCategory::MATERIALS);

    myRootSignaturePool.clear();
  }
//---------------------------------------------------------------------------//
}}}

#endif // RENDERER_DX12