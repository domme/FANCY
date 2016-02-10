#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "RenderContext.h"

#if defined (RENDERER_DX12)

#include "FenceDX12.h"
#include "RenderContextDX12.h"

namespace Fancy { namespace Rendering {
  class GeometryVertexLayout;
} }

namespace Fancy { namespace Rendering { namespace DX12 {
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
    bool IsFenceDone(uint64 aFrameDoneFenceVal) { return mySyncFence.IsDone(aFrameDoneFenceVal); }
    ID3D12Device* GetDevice() const { return myDevice.Get(); }

    RenderContext& GetDefaultContext() { return myDefaultContext; }

    uint64 ExecuteCommandList(ID3D12CommandList* aCommandList);

	protected:
    void init(void* aNativeWindowHandle);

    RenderContext myDefaultContext; 

    // Synchronization objects.
    uint myCurrBackbufferIndex;
    FenceDX12 mySyncFence;

    static const uint kBackbufferCount = 2u;
    ComPtr<ID3D12Device> myDevice;
    ComPtr<IDXGISwapChain3> mySwapChain;
    ComPtr<ID3D12Resource> myBackbuffers[kBackbufferCount];
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