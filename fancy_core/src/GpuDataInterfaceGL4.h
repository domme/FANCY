#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

#if defined(RENDERER_GL4)
//---------------------------------------------------------------------------//
  namespace Fancy { namespace Rendering {
      class MaterialPassInstance;
      class Renderer;
      class MaterialPass;
  } }
//---------------------------------------------------------------------------//
  namespace Fancy { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//
  class GpuDataInterfaceGL4
  {
  public:
    GpuDataInterfaceGL4() {};
    ~GpuDataInterfaceGL4() {}

    void applyMaterialPass(const MaterialPass* _pMaterialPass, Renderer* _pRenderer);
    void applyMaterialPassInstance(const MaterialPassInstance* _pMaterialPassInstance, Renderer* _pRenderer);
  };
//---------------------------------------------------------------------------//
} } } // namespace Fancy { namespace Rendering { namespace GL4 {
#endif // RENDERER_GL4