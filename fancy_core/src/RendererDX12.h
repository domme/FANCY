#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "RenderContext.h"
#include "DataFormat.h"
#include "CommandListType.h"

#if defined (RENDERER_DX12)

#include "Texture.h"
#include "FenceDX12.h"
#include "RenderContextDX12.h"
#include "CommandAllocatorPoolDX12.h"

namespace Fancy { namespace Rendering {
class ShaderResourceInterface;
class GeometryVertexLayout;
} }

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class DescriptorHeapPoolDX12;
//---------------------------------------------------------------------------//
	class RenderOutputDX12
	{
	public:
    RenderOutputDX12(void* aNativeWindowHandle);
		virtual ~RenderOutputDX12();
		void postInit(); /// Sets the render-system to a valid state. Should be called just before the first frame
		void beginFrame();
		void endFrame();

    Texture* GetBackbuffer() { return &myBackbuffers[myCurrBackbufferIndex]; }
    Texture* GetDefaultDepthStencilBuffer() { return &myDefaultDepthStencil; }

	protected:
    void CreateSwapChain(void* aNativeWindowHandle);
    void CreateBackbufferResources();

    uint myCurrBackbufferIndex;
    
    static const uint kBackbufferCount = 2u;
    ComPtr<IDXGISwapChain3> mySwapChain;
    Texture myBackbuffers[kBackbufferCount];
    Texture myDefaultDepthStencil;
	};
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//

  // TODO: Make the RenderCore the main API entry point for render-related stuff (avoid having to call Fancy::GetRenderer() everywhere)

  class RenderCoreDX12
  {
  public:
    /// Initializes platform-dependent rendering stuff
    static void InitPlatform();
    /// Shutdown of platform-dependent rendering stuff
    static void ShutdownPlatform();

    static DXGI_FORMAT GetFormat(DataFormat aFormat);
    static DataFormat ResolveFormat(DataFormat aFormat);
    static DataFormatInfo GetFormatInfo(DXGI_FORMAT aFormat);

    static Rendering::ShaderResourceInterface* 
      GetShaderResourceInterface(const D3D12_ROOT_SIGNATURE_DESC& anRSdesc, ComPtr<ID3D12RootSignature> anRS = nullptr);

    static void WaitForFence(CommandListType aType, uint64 aFenceVal);
    static void IsFenceDone(CommandListType aType, uint64 aFenceVal);

    static uint64 ExecuteCommandList(ID3D12CommandList* aCommandList);
    
    static CommandAllocatorPoolDX12* GetCommandAllocatorPool() { return ourCommandAllocatorPool; }
    static DescriptorDX12 AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType);
    static DescriptorHeapPoolDX12* GetDescriptorHeapPool() { return ourDescriptorHeapPool; }

    static CommandAllocatorPoolDX12* ourCommandAllocatorPool;
    static DescriptorHeapPoolDX12* ourDescriptorHeapPool;
    static ComPtr<ID3D12Device> ourDevice;

    static ComPtr<ID3D12CommandQueue> ourCommandQueues[(uint) CommandListType::NUM];
    static FenceDX12 ourCmdListDoneFences[(uint)CommandListType::NUM];

  private:
    RenderCoreDX12() {}
  };
//---------------------------------------------------------------------------//
} } } // end of namespace Fancy::Renderer::DX12

#endif