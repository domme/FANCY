#include "RenderOutputDX12.h"
#include "RenderCore_PlatformDX12.h"
#include "RenderCore.h"
#include "Window.h"
#include "TextureDX12.h"
#include "BlendState.h"
#include "CommandContextDX12.h"
#include "GpuResourceStorageDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  RenderOutputDX12::RenderOutputDX12(void* aNativeInstanceHandle)
    : RenderOutput(aNativeInstanceHandle)
  {
    CreateSwapChain();
  }
  //---------------------------------------------------------------------------//
  RenderOutputDX12::~RenderOutputDX12()
  {
    mySwapChain.Reset();
  }
//---------------------------------------------------------------------------//
  void RenderOutputDX12::PrepareForFirstFrame()
  {
    CreateBackbufferResources();
  }
//---------------------------------------------------------------------------//
  void RenderOutputDX12::BeginFrame()
  {
    CommandQueueDX12* graphicsQueue = static_cast<CommandQueueDX12*>(RenderCore::GetCommandQueue(CommandListType::Graphics));
    graphicsQueue->WaitForIdle();  // Wait for the last frame to finish - needed?
    myCurrBackbufferIndex = mySwapChain->GetCurrentBackBufferIndex();
  }
//---------------------------------------------------------------------------//
  void RenderOutputDX12::EndFrame()
  {
    Texture* currBackbuffer = myBackbufferRtv[myCurrBackbufferIndex]->GetTexture();

    CommandQueueDX12* graphicsQueue = static_cast<CommandQueueDX12*>(RenderCore::GetCommandQueue(CommandListType::Graphics));
    CommandContextDX12* context = static_cast<CommandContextDX12*>(RenderCore::AllocateContext(CommandListType::Graphics));
    
    context->TransitionResource(currBackbuffer, GpuResourceState::RESOURCE_STATE_PRESENT);
    graphicsQueue->ExecuteContext(context);
    
    RenderCore::FreeContext(context);

    mySwapChain->Present(1, 0);
  }
//---------------------------------------------------------------------------//
  void RenderOutputDX12::CreateSwapChain()
  {
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = kBackbufferCount;
    swapChainDesc.BufferDesc.Width = myWindow->GetWidth();
    swapChainDesc.BufferDesc.Height = myWindow->GetHeight();
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.OutputWindow = myWindow->GetWindowHandle();
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;

    RenderCore_PlatformDX12* coreDX12 = static_cast<RenderCore_PlatformDX12*>(RenderCore::GetPlatform());
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain = coreDX12->CreateSwapChain(swapChainDesc);
    ASSERT(swapChain != nullptr);

    CheckD3Dcall(swapChain.As(&mySwapChain));
    myCurrBackbufferIndex = mySwapChain->GetCurrentBackBufferIndex();
  }
//---------------------------------------------------------------------------//
  void RenderOutputDX12::OnWindowResized(uint aWidth, uint aHeight)
  {
    CheckD3Dcall(mySwapChain->ResizeBuffers(kBackbufferCount, aWidth, aHeight, DXGI_FORMAT_UNKNOWN, 0));
    RenderOutput::OnWindowResized(aWidth, aHeight);
  }
  //---------------------------------------------------------------------------//
}
