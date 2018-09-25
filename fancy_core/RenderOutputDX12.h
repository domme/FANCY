#pragma once

#include "DX12Prerequisites.h"
#include "RenderOutput.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class Window;
//---------------------------------------------------------------------------//
  class RenderOutputDX12 : public RenderOutput
  {
  public:
    explicit RenderOutputDX12(void* aNativeInstanceHandle);
    ~RenderOutputDX12() override;
    
    void BeginFrame() override;
    void EndFrame() override;

  private:
    void CreateBackbufferResources(uint aWidth, uint aHeight) override;
    void ResizeBackbuffer(uint aWidth, uint aHeight) override;
    void DestroyBackbufferResources() override;

    Microsoft::WRL::ComPtr<IDXGISwapChain3> mySwapChain;
  };
//---------------------------------------------------------------------------//
}
