#pragma once

#include "RendererPrerequisites.h"

#if defined (RENDERER_DX12)
#include "FancyCorePrerequisites.h"

//---------------------------------------------------------------------------//
namespace Fancy { namespace Rendering {
  class GpuResourceSignature;
  class MaterialPassInstance;
  class Renderer;
  class MaterialPass;
}}
//---------------------------------------------------------------------------//
namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class GpuDataInterfaceDX12
  {
    public:
      explicit GpuDataInterfaceDX12(const std::vector<GpuResourceSignature>& someGpuResourceSignatures);
      ~GpuDataInterfaceDX12() {}

      void SetGpuResourceSignature(const GpuResourceSignature& aSignature);
      void applyMaterialPass(const MaterialPass* _pMaterialPass, Renderer* _pRenderer);
      void applyMaterialPassInstance(const MaterialPassInstance* _pMaterialPassInstance, Renderer* _pRenderer);

    protected:
      std::vector<uint> myGpuResourceSignatureHashes;
      std::vector<ComPtr<ID3D12RootSignature>> myRootSignatures;
  };
//---------------------------------------------------------------------------//
}}} // namespace Fancy { namespace Rendering { namespace DX12 {

#endif  // RENDERER_DX12