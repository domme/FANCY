#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

#if defined (RENDERER_DX12)

#include "DepthStencilState.h"
#include "BlendState.h"
#include "FenceDX12.h"
#include <unordered_map>

namespace Fancy { namespace Rendering {
  class GeometryVertexLayout;
} }

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
	class RendererDX12
	{
	public:
		virtual ~RendererDX12();
    void init(void* aNativeWindowHandle);
		void postInit(); /// Sets the render-system to a valid state. Should be called just before the first frame
		void beginFrame();
		void endFrame();

    bool IsFrameDone(uint64 aFrameDoneFenceVal) { return myFrameDone.IsDone(aFrameDoneFenceVal); }
    ComPtr<ID3D12Device>& GetDevice() { return myDevice; }

	protected:
	  RendererDX12();

    // Synchronization objects.
    uint myCurrBackbufferIndex;
    FenceDX12 myFrameDone;

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