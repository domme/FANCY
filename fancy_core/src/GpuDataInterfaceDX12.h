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
      explicit GpuDataInterfaceDX12();
      ~GpuDataInterfaceDX12() {}

      void applyMaterialPass(const MaterialPass* _pMaterialPass, Renderer* _pRenderer);
      void applyMaterialPassInstance(const MaterialPassInstance* _pMaterialPassInstance, Renderer* _pRenderer);
  };
//---------------------------------------------------------------------------//
}}} // namespace Fancy { namespace Rendering { namespace DX12 {

#endif  // RENDERER_DX12