#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "RenderContext.h"
#include "DataFormat.h"

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
	class RendererDX12
	{
	public:
    RendererDX12(void* aNativeWindowHandle);
		virtual ~RendererDX12();
		void postInit(); /// Sets the render-system to a valid state. Should be called just before the first frame
		void beginFrame();
		void endFrame();

    void WaitForFence(uint64 aFenceVal);
    bool IsFenceDone(uint64 aFrameDoneFenceVal) { return myFence.IsDone(aFrameDoneFenceVal); }
    ID3D12Device* GetDevice() const { return myDevice.Get(); }

    CommandAllocatorPoolDX12* GetCommandAllocatorPool() { return myCommandAllocatorPool; }
    RenderContext* GetDefaultContext() { return myDefaultContext; }
    
    DescriptorDX12 AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType);

    DescriptorHeapPoolDX12* GetDescriptorHeapPool() { return myDescriptorHeapPool; }
    uint64 ExecuteCommandList(ID3D12CommandList* aCommandList);

    Texture* GetBackbuffer() { return &myBackbuffers[myCurrBackbufferIndex]; }
    Texture* GetDefaultDepthStencilBuffer() { return &myDefaultDepthStencil; }

	protected:
    void CreateDeviceAndSwapChain(void* aNativeWindowHandle);
    void CreateBackbufferResources();

    RenderContext* myDefaultContext; 
    CommandAllocatorPoolDX12* myCommandAllocatorPool;
    DescriptorHeapPoolDX12* myDescriptorHeapPool;

    FenceDX12 myFence;

    uint myCurrBackbufferIndex;
    
    static const uint kBackbufferCount = 2u;
    ComPtr<ID3D12Device> myDevice;
    ComPtr<IDXGISwapChain3> mySwapChain;
    Texture myBackbuffers[kBackbufferCount];
    Texture myDefaultDepthStencil;
    ComPtr<ID3D12CommandQueue> myCommandQueue;
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
  };
//---------------------------------------------------------------------------//
} } } // end of namespace Fancy::Renderer::DX12

#endif