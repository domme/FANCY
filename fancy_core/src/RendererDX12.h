#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "RenderContext.h"

#if defined (RENDERER_DX12)

#include "TextureDX12.h"
#include "FenceDX12.h"
#include "RenderContextDX12.h"
#include "CommandAllocatorPoolDX12.h"

namespace Fancy { namespace Rendering {
  class GeometryVertexLayout;
} }

namespace Fancy { namespace Rendering { namespace DX12 {
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
    
    DescriptorHeapPoolDX12* GetDescriptorHeapPool() { return myDescriptorHeapPool; }
    uint64 ExecuteCommandList(ID3D12CommandList* aCommandList);

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
    TextureDX12 myBackbuffers[kBackbufferCount];
    ComPtr<ID3D12CommandQueue> myCommandQueue;
	};
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  class RenderingSubsystemDX12
  {
  public:
    /// Initializes platform-dependent rendering stuff
    static void InitPlatform();
    /// Shutdown of platform-dependent rendering stuff
    static void ShutdownPlatform();
  };
//---------------------------------------------------------------------------//
} } } // end of namespace Fancy::Renderer::DX12

#endif