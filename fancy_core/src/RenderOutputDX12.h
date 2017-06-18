#pragma once

#include "DX12Prerequisites.h"
#include "RenderOutput.h"

namespace Fancy {
  class RenderWindow;
}

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class RenderOutputDX12 : public RenderOutput
  {
  public:
    explicit RenderOutputDX12(void* aNativeInstanceHandle);
    ~RenderOutputDX12() override;
    
    void PrepareForFirstFrame() override;
    void BeginFrame() override;
    void EndFrame() override;

  private:
    void OnWindowResized(uint aWidth, uint aHeight) override;
    void CreateBackbufferResources() override;

    void CreateSwapChain();
    void DestroyBackbufferResources();

    Microsoft::WRL::ComPtr<IDXGISwapChain3> mySwapChain;
  };
//---------------------------------------------------------------------------//
} } }
