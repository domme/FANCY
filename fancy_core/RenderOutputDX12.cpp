#include "RenderOutputDX12.h"
#include "RenderCore_PlatformDX12.h"
#include "RenderCore.h"
#include "Window.h"
#include "TextureDX12.h"
#include "CommandContextDX12.h"
#include "GpuResourceStorageDX12.h"
#include "StringUtil.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  RenderOutputDX12::RenderOutputDX12(void* aNativeInstanceHandle, const WindowParameters& someWindowParams)
    : RenderOutput(aNativeInstanceHandle, someWindowParams)
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
  RenderOutputDX12::~RenderOutputDX12()
  {
  }
//---------------------------------------------------------------------------//
  void RenderOutputDX12::CreateBackbufferResources(uint aWidth, uint aHeight)
  {
    for (uint i = 0u; i < kBackbufferCount; ++i)
    {
      GpuResource resource(GpuResourceCategory::TEXTURE);
      resource.myName = "Backbuffer Texture " + i;

      {
        UniquePtr<GpuResourceStorageDX12> resourceStorage(new GpuResourceStorageDX12);
        resourceStorage->mySubresourceStates.push_back(D3D12_RESOURCE_STATE_PRESENT);
        resourceStorage->mySubresourceContexts.push_back(CommandListType::Graphics);
        resourceStorage->myAllSubresourcesInSameState = true;
        resourceStorage->myReadState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        CheckD3Dcall(mySwapChain->GetBuffer(i, IID_PPV_ARGS(&resourceStorage->myResource)));
        std::wstring wName = StringUtil::ToWideString(resource.myName);
        resourceStorage->myResource->SetName(wName.c_str());
        
        resource.myStorage = std::move(resourceStorage);  
      }

      TextureProperties backbufferProps;
      backbufferProps.myDimension = GpuResourceDimension::TEXTURE_2D;
      backbufferProps.myIsRenderTarget = true;
      backbufferProps.eFormat = DataFormat::RGBA_8;
      backbufferProps.myWidth = aWidth;
      backbufferProps.myHeight = aHeight;
      backbufferProps.myDepthOrArraySize = 1u;
      backbufferProps.myNumMipLevels = 1u;

      myBackbufferTextures[i].reset(new TextureDX12());
      myBackbufferTextures[i]->Create(std::move(resource), backbufferProps);
    }
  }
//---------------------------------------------------------------------------//
  void RenderOutputDX12::BeginFrame()
  {
    myCurrBackbufferIndex = mySwapChain->GetCurrentBackBufferIndex();
  }
//---------------------------------------------------------------------------//
  void RenderOutputDX12::EndFrame()
  {
    Texture* currBackbuffer = myBackbufferRtv[myCurrBackbufferIndex]->GetTexture();

    CommandQueueDX12* graphicsQueue = static_cast<CommandQueueDX12*>(RenderCore::GetCommandQueue(CommandListType::Graphics));
    CommandContextDX12* context = static_cast<CommandContextDX12*>(RenderCore::AllocateContext(CommandListType::Graphics));

    context->TransitionResource(currBackbuffer, GpuResourceTransition::TO_PRESENT);
    graphicsQueue->ExecuteContext(context);
    RenderCore::FreeContext(context);

    graphicsQueue->WaitForIdle();
    mySwapChain->Present(1, 0);
  }
//---------------------------------------------------------------------------//
  void RenderOutputDX12::ResizeBackbuffer(uint aWidth, uint aHeight)
  {
    CheckD3Dcall(mySwapChain->ResizeBuffers(kBackbufferCount, aWidth, aHeight, DXGI_FORMAT_UNKNOWN, 0));
  }
//---------------------------------------------------------------------------//
  void RenderOutputDX12::DestroyBackbufferResources()
  {
    for (uint i = 0u; i < kBackbufferCount; ++i)
    {
      myBackbufferTextures[i].reset();
    }
  }
//---------------------------------------------------------------------------//
}
