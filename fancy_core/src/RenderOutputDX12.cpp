#include "RenderOutputDX12.h"
#include "RenderCore_PlatformDX12.h"
#include "RenderCore.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  RenderOutputDX12::RenderOutputDX12(void* aNativeInstanceHandle)
  {
    

    CreateSwapChain();
  }
  //---------------------------------------------------------------------------//
  RenderOutputDX12::~RenderOutputDX12()
  {
    
  }
  //---------------------------------------------------------------------------//
  void RenderOutputDX12::CreateSwapChain()
  {
    using namespace Microsoft::WRL;

    HWND windowHandle = myWindow->GetWindowHandle();

    ComPtr<IDXGIFactory4> dxgiFactory;
    CheckD3Dcall(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = kBackbufferCount;
    swapChainDesc.BufferDesc.Width = myWindow->GetWidth();
    swapChainDesc.BufferDesc.Height = myWindow->GetHeight();
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.OutputWindow = windowHandle;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;

    ComPtr<IDXGISwapChain> swapChain;
    CheckD3Dcall(dxgiFactory->CreateSwapChain(RenderCore::ourCommandQueues[(uint)CommandListType::Graphics].Get(), &swapChainDesc, &swapChain));
    CheckD3Dcall(swapChain.As(&mySwapChain));
    myCurrBackbufferIndex = mySwapChain->GetCurrentBackBufferIndex();
  }
  //---------------------------------------------------------------------------//
  void RenderOutputDX12::DestroyBackbufferResources()
  {
    for (uint i = 0u; i < kBackbufferCount; ++i)
      myBackbuffers[i] = nullptr;

    myDefaultDepthStencil = nullptr;
  }
  //---------------------------------------------------------------------------//
  void RenderOutputDX12::CreateBackbufferResources()
  {
    RenderOutput::CreateBackbufferResources();

    // TODO: Remove this hack and let this be handled internally by the 
    // core platform abstraction layer

    RenderCore_PlatformDX12* coreDX12 = static_cast<RenderCore_PlatformDX12*>(RenderCore::GetPlatformImpl());

    for (uint32 i = 0u; i < kBackbufferCount; i++)
    {
      TextureDX12* backbufferResource = (TextureDX12*)myBackbuffers[i].get();
      backbufferResource->myUsageState = D3D12_RESOURCE_STATE_PRESENT;

      // TODO: Sync this better with swap chain properties
      backbufferResource->myParameters.myIsRenderTarget = true;
      

      CheckD3Dcall(mySwapChain->GetBuffer(n, IID_PPV_ARGS(&backbufferResource->myResource)));
      backbufferResource->myRtvDescriptor = RenderCoreDX12::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
      RenderCoreDX12::ourDevice->CreateRenderTargetView(backbufferResource->myResource.Get(), nullptr, backbufferResource->myRtvDescriptor.myCpuHandle);
    }
  }
  //---------------------------------------------------------------------------//
  void RenderOutputDX12::postInit()
  {
    CreateBackbufferResources();
  }
  //---------------------------------------------------------------------------//
  void RenderOutputDX12::OnWindowResized(uint aWidth, uint aHeight)
  {
    DestroyBackbufferResources();
    CheckD3Dcall(mySwapChain->ResizeBuffers(kBackbufferCount, aWidth, aHeight, DXGI_FORMAT_UNKNOWN, 0));
    CreateBackbufferResources();
  }
  //---------------------------------------------------------------------------//
  void RenderOutputDX12::beginFrame()
  {
    RenderCoreDX12::ourCmdListDoneFences[(uint)CommandListType::Graphics].wait();  // Needed?
    myCurrBackbufferIndex = mySwapChain->GetCurrentBackBufferIndex();
  }
  //---------------------------------------------------------------------------//
  void RenderOutputDX12::endFrame()
  {
    TextureDX12* currBackbuffer = myBackbuffers[myCurrBackbufferIndex];

    CommandContext* context = CommandContext::AllocateContext(CommandListType::Graphics);
    context->TransitionResource(currBackbuffer, D3D12_RESOURCE_STATE_PRESENT, true);
    context->ExecuteAndReset(false);
    CommandContext::FreeContext(context);

    mySwapChain->Present(1, 0);
  }
//---------------------------------------------------------------------------//
} } }
