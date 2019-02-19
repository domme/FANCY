#pragma once

#include "FancyCoreDefines.h"
#include "Ptr.h"
#include "CircularArray.h"

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
    explicit RenderOutput(void* aNativeInstanceHandle, const WindowParameters& someWindowParams);
    virtual ~RenderOutput();
    
    virtual void BeginFrame() = 0;
    
    void PrepareForFirstFrame();
    void EndFrame();

    TextureView* GetBackbufferRtv() const { return myBackbufferRtv[myCurrBackbufferIndex].get(); }
    TextureView* GetBackbufferSrv() const { return myBackbufferSrv[myCurrBackbufferIndex].get(); }
    TextureView* GetDepthStencilDsv() const { return myDepthStencilDsv.get(); }
    TextureView* GetDepthStencilDsv_ReadOnly() const { return myDepthStencilDsv_ReadOnly.get(); }
    TextureView* GetDepthSrv() const { return myDepthSrv.get(); }
    Window* GetWindow() const { return myWindow.get(); }

  protected:
    void GetWindowSizeSafe(uint& aWidthOut, uint& aHeightOut);
    void OnWindowResized(uint aWidth, uint aHeight);

    virtual void CreateBackbufferResources(uint aWidth, uint aHeight) = 0;
    virtual void ResizeBackbuffer(uint aWidth, uint aHeight) = 0;
    virtual void DestroyBackbufferResources() = 0;
    virtual void Present() = 0;

    void DestroyViews();
    void CreateViews();

    static const uint kBackbufferCount = 2u;
    static const uint kMaxFrameDelay = 3u;

    uint64 myNextWaitFence = 0u;
    uint64 myLastWaitedOnFrame = 0u;

    uint myCurrBackbufferIndex;
    SharedPtr<Window> myWindow;

    SharedPtr<Texture> myBackbufferTextures[kBackbufferCount];
    SharedPtr<TextureView> myBackbufferRtv[kBackbufferCount];
    SharedPtr<TextureView> myBackbufferSrv[kBackbufferCount];
    SharedPtr<TextureView> myDepthStencilDsv;
    SharedPtr<TextureView> myDepthStencilDsv_ReadOnly;
    SharedPtr<TextureView> myDepthSrv;
  };
//---------------------------------------------------------------------------//
}
