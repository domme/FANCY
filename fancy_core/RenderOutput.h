#pragma once

#include "FancyCorePrerequisites.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct GpuProgramDesc;
  struct GpuProgramPipelineDesc;
  struct DepthStencilStateDesc;
  struct BlendStateDesc;
  class BlendState;
  class DepthStencilState;
  class Window;
  class Texture;
  class TextureView;
//---------------------------------------------------------------------------//
  class RenderOutput
  {
  public:
    explicit RenderOutput(void* aNativeInstanceHandle);
    virtual ~RenderOutput();

    virtual void PrepareForFirstFrame() = 0;
    virtual void BeginFrame() = 0;
    virtual void EndFrame()= 0;

    TextureView* GetBackbufferRtv() const { return myBackbufferRtv[myCurrBackbufferIndex].get(); }
    TextureView* GetBackbufferSrv() const { return myBackbufferSrv[myCurrBackbufferIndex].get(); }
    TextureView* GetDepthStencilDsv() const { return myDepthStencilDsv.get(); }
    TextureView* GetDepthStencilDsv_ReadOnly() const { return myDepthStencilDsv_ReadOnly.get(); }
    TextureView* GetDepthSrv() const { return myDepthSrv.get(); }
    Window* GetWindow() const { return myWindow.get(); }

  protected:
    virtual void OnWindowResized(uint aWidth, uint aHeight) = 0;
    virtual void CreateBackbufferResources();

    static const uint kBackbufferCount = 2u;
    uint myCurrBackbufferIndex;
    SharedPtr<Window> myWindow;

    SharedPtr<TextureView> myBackbufferRtv[kBackbufferCount];
    SharedPtr<TextureView> myBackbufferSrv[kBackbufferCount];
    SharedPtr<TextureView> myDepthStencilDsv;
    SharedPtr<TextureView> myDepthStencilDsv_ReadOnly;
    SharedPtr<TextureView> myDepthSrv;
  };
//---------------------------------------------------------------------------//
}