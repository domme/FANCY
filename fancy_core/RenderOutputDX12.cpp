#include "fancy_core_precompile.h"
#include "RenderOutputDX12.h"

#include "RenderCore_PlatformDX12.h"
#include "RenderCore.h"
#include "Window.h"
#include "TextureDX12.h"
#include "CommandListDX12.h"
#include "GpuResourceDataDX12.h"

#if FANCY_ENABLE_DX12

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

    ASSERT_HRESULT(swapChain.As(&mySwapChain));
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
      GpuResource resource(GpuResourceType::TEXTURE);
      resource.myName = StaticString<32>("Backbuffer Texture %d", i);

      GpuResourceDataDX12* dataDx12(new GpuResourceDataDX12);
      ASSERT_HRESULT(mySwapChain->GetBuffer(i, IID_PPV_ARGS(&dataDx12->myResource)));
      std::wstring wName = StringUtil::ToWideString(resource.myName);
      dataDx12->myResource->SetName(wName.c_str());

      GpuSubresourceHazardDataDX12 subHazardData;
      subHazardData.myContext = CommandListType::Graphics;
      subHazardData.myStates = D3D12_RESOURCE_STATE_PRESENT;

      resource.mySubresources = SubresourceRange(0u, 1u, 0u, 1u, 0u, 1u);

      GpuResourceHazardDataDX12* hazardData = &dataDx12->myHazardData;
      hazardData->myReadStates = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_COPY_SOURCE;
      hazardData->myWriteStates = D3D12_RESOURCE_STATE_RENDER_TARGET | D3D12_RESOURCE_STATE_COPY_DEST;
      hazardData->mySubresources.push_back(subHazardData);
      hazardData->myCanChangeStates = true;

      resource.myNativeData = dataDx12;

      TextureProperties backbufferProps;
      backbufferProps.myDimension = GpuResourceDimension::TEXTURE_2D;
      backbufferProps.myIsRenderTarget = true;
      backbufferProps.myFormat = DataFormat::RGBA_8;
      backbufferProps.myWidth = aWidth;
      backbufferProps.myHeight = aHeight;
      backbufferProps.myDepthOrArraySize = 1u;
      backbufferProps.myNumMipLevels = 1u;

      myBackbufferTextures[i].reset(new TextureDX12(std::move(resource), backbufferProps, true));
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
    CommandList* ctx = RenderCore::BeginCommandList(CommandListType::Graphics);
    CommandListDX12* ctxDx12 = static_cast<CommandListDX12*>(ctx);
    ctxDx12->TrackResourceTransition(GetBackbuffer(), D3D12_RESOURCE_STATE_PRESENT);
    RenderCore::ExecuteAndFreeCommandList(ctx);

    mySwapChain->Present(1, 0);
  }
//---------------------------------------------------------------------------//
  void RenderOutputDX12::ResizeSwapChain(uint aWidth, uint aHeight)
  {
    ASSERT_HRESULT(mySwapChain->ResizeBuffers(kBackbufferCount, aWidth, aHeight, DXGI_FORMAT_UNKNOWN, 0));
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

#endif