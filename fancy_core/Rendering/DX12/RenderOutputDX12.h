#pragma once

#include "DX12Prerequisites.h"
#include "Rendering/RenderOutput.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
//---------------------------------------------------------------------------//
  class Window;
//---------------------------------------------------------------------------//
  class RenderOutputDX12 final : public RenderOutput
  {
  public:
    RenderOutputDX12(void* aNativeInstanceHandle, const WindowParameters& someWindowParams);
    ~RenderOutputDX12() override;
    
  private:
    void CreateBackbufferResources(uint aWidth, uint aHeight) override;
    void ResizeSwapChain(uint aWidth, uint aHeight) override;
    void DestroyBackbufferResources() override;

    void OnBeginFrame() override;
    void Present() override;

    Microsoft::WRL::ComPtr<IDXGISwapChain3> mySwapChain;
  };
//---------------------------------------------------------------------------//
}

#endif