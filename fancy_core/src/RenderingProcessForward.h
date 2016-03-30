#ifndef INCLUDE_RENDERINGPROCESSFORWARD_H
#define INCLUDE_RENDERINGPROCESSFORWARD_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "RenderingProcess.h"

//---------------------------------------------------------------------------//
namespace Fancy { namespace Rendering {
class GpuDataInterface;

  class DLLEXPORT RenderingProcessForward : public RenderingProcess
  {
  public:
    RenderingProcessForward();
    virtual ~RenderingProcessForward();

    virtual void startup() override;
    virtual void tick(float _dt) override;

  private:
    std::shared_ptr<GpuDataInterface> myGpuDataInterface;


  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(RenderingProcessForward)
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering

#endif  // INCLUDE_RENDERINGPROCESS_H