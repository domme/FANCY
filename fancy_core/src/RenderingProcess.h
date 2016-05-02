#ifndef INCLUDE_RENDERINGPROCESS_H
#define INCLUDE_RENDERINGPROCESS_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

//---------------------------------------------------------------------------//
namespace Fancy { namespace Rendering {
class RenderContext;
class MaterialPass;

class DLLEXPORT RenderingProcess
  {
  public:
    RenderingProcess();
    virtual ~RenderingProcess();

    virtual void Startup() = 0;
    virtual void Tick(float _dt) = 0;

  protected:
    static void ApplyMaterialPass(const MaterialPass* _pMaterialPass, RenderContext* aRenderContext);

  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(RenderingProcess)
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering

#endif  // INCLUDE_RENDERINGPROCESS_H