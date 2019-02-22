#include "fancy_core_precompile.h"
#include "RenderOutputDX12.h"

#include "RenderCore_PlatformDX12.h"
#include "RenderCore.h"
#include "Window.h"
#include "TextureDX12.h"
#include "CommandContextDX12.h"
#include "GpuResourceDataDX12.h"

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
        GpuResourceDataDX12* dataDx12(new GpuResourceDataDX12);
        CheckD3Dcall(mySwapChain->GetBuffer(i, IID_PPV_ARGS(&dataDx12->myResource)));
        std::wstring wName = StringUtil::ToWideString(resource.myName);
        dataDx12->myResource->SetName(wName.c_str());
        resource.myNativeData = dataDx12;
      }

      resource.myHazardData.reset(new GpuHazardDataDX12);
      GpuHazardDataDX12* hazardDataDx12 = static_cast<GpuHazardDataDX12*>(resource.myHazardData.get());
      hazardDataDx12->mySubresourceStates.push_back(D3D12_RESOURCE_STATE_PRESENT);
      hazardDataDx12->mySubresourceContexts.push_back(CommandListType::Graphics);
      hazardDataDx12->myAllSubresourcesInSameState = true;
      hazardDataDx12->myReadState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

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
  void RenderOutputDX12::OnBeginFrame()
  {
    myCurrBackbufferIndex = mySwapChain->GetCurrentBackBufferIndex();
  }
//---------------------------------------------------------------------------//
  void RenderOutputDX12::Present()
  {
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
