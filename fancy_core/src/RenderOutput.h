#pragma once
#include "ScopedPtr.h"

namespace Fancy {
  class RenderWindow;
}

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct GpuProgramDesc;
  struct GpuProgramPipelineDesc;
  struct DepthStencilStateDesc;
  struct BlendStateDesc;
  class BlendState;
  class DepthStencilState;
//---------------------------------------------------------------------------//
  class DLLEXPORT RenderOutput
  {
  public:
    explicit RenderOutput(void* aNativeInstanceHandle);
    virtual ~RenderOutput();

    virtual void PrepareForFirstFrame();
    virtual void BeginFrame();
    virtual void EndFrame();

  protected:
    virtual void OnWindowResized(uint aWidth, uint aHeight);

    static const uint kBackbufferCount = 2u;

    uint myCurrBackbufferIndex;
    SharedPtr<RenderWindow> myWindow;
    ScopedPtr<Texture> myBackbuffers[kBackbufferCount];
    ScopedPtr<Texture> myDefaultDepthStencil;
  };
//---------------------------------------------------------------------------//
} }