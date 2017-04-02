#pragma once

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

    Texture* GetBackbuffer() const { return myBackbuffers[myCurrBackbufferIndex].get(); }
    Texture* GetDefaultDepthStencilBuffer() const { return myDefaultDepthStencil.get(); }
    RenderWindow* GetWindow() const { return myWindow.get(); }

  protected:
    virtual void OnWindowResized(uint aWidth, uint aHeight);

    static const uint kBackbufferCount = 2u;

    uint myCurrBackbufferIndex;
    SharedPtr<RenderWindow> myWindow;
    SharedPtr<Texture> myBackbuffers[kBackbufferCount];
    SharedPtr<Texture> myDefaultDepthStencil;
  };
//---------------------------------------------------------------------------//
} }