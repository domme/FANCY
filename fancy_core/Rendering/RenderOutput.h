#pragma once

#include "Common/FancyCoreDefines.h"
#include "Common/Ptr.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct WindowParameters;
  class Window;
  class Texture;
  class TextureView;
//---------------------------------------------------------------------------//
  class RenderOutput
  {
  public:
    RenderOutput(void* aNativeInstanceHandle, const WindowParameters& someWindowParams);
    virtual ~RenderOutput();
    
    void BeginFrame();
    void EndFrame();

    Texture* GetBackbuffer() const { return myBackbufferTextures[myCurrBackbufferIndex].get(); }
    TextureView* GetBackbufferRtv() const { return myBackbufferRtv[myCurrBackbufferIndex].get(); }
    TextureView* GetBackbufferSrv() const { return myBackbufferSrv[myCurrBackbufferIndex].get(); }
    Window* GetWindow() const { return myWindow.get(); }

  protected:
    void PrepareForFirstFrame();
    void GetWindowSizeSafe(uint& aWidthOut, uint& aHeightOut);
    void OnWindowResized(uint aWidth, uint aHeight);

    virtual void CreateBackbufferResources(uint aWidth, uint aHeight) = 0;
    virtual void ResizeSwapChain(uint aWidth, uint aHeight) = 0;
    virtual void DestroyBackbufferResources() = 0;

    virtual void OnBeginFrame() = 0;
    virtual void Present() = 0;

    void DestroyViews();
    void CreateViews();

    static const uint kBackbufferCount = 2u;

    uint myCurrBackbufferIndex;
    SharedPtr<Window> myWindow;

    SharedPtr<Texture> myBackbufferTextures[kBackbufferCount];
    SharedPtr<TextureView> myBackbufferRtv[kBackbufferCount];
    SharedPtr<TextureView> myBackbufferSrv[kBackbufferCount];
  };
//---------------------------------------------------------------------------//
}
