#ifndef INCLUDE_RENDERINGPROCESS_H
#define INCLUDE_RENDERINGPROCESS_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

//---------------------------------------------------------------------------//
namespace Fancy { namespace Rendering {

  class DLLEXPORT RenderingProcess
  {
  public:
    RenderingProcess();
    virtual ~RenderingProcess();

    virtual void startup() = 0;
    virtual void tick(float _dt) = 0;
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(RenderingProcess)
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering

#endif  // INCLUDE_RENDERINGPROCESS_H