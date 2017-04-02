#pragma once

#include "DX12Prerequisites.h"
#include "ScopedPtr.h"
#include "RenderOutput.h"

namespace Fancy {
  class RenderWindow;
}

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class RenderOutputDX12 : public RenderOutput
  {
  public:
    RenderOutputDX12(void* aNativeInstanceHandle);
    virtual ~RenderOutputDX12();
    void postInit(); /// Sets the render-system to a valid state. Should be called just before the first frame
    void beginFrame();
    void endFrame();

    Texture* GetBackbuffer() const { return myBackbuffers[myCurrBackbufferIndex]; }
    Texture* GetDefaultDepthStencilBuffer() const { return myDefaultDepthStencil; }
    RenderWindow* GetWindow();
    const RenderWindow* GetWindow() const;

  protected:
    void CreateSwapChain();
    void DestroyBackbufferResources();
    void CreateBackbufferResources();

    ComPtr<IDXGISwapChain3> mySwapChain;
    
  };
//---------------------------------------------------------------------------//
} } }
