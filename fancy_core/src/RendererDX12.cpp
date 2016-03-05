#include "RendererDX12.h"
#include "AdapterDX12.h"

#if defined (RENDERER_DX12)
#include "MathUtil.h"
#include "GpuProgram.h"
#include "RootSignatureDX12.h"
#include "GpuProgramCompiler.h"
#include "DescriptorHeapPoolDX12.h"
#include "Renderer.h"
#include "RenderContext.h"

namespace Fancy { namespace Rendering { namespace DX12 { 

//---------------------------------------------------------------------------//
  RendererDX12::RendererDX12(void* aNativeWindowHandle)
    : myCommandAllocatorPool(nullptr)
    , myDefaultContext(nullptr)
	{
    CreateDeviceAndSwapChain(aNativeWindowHandle);
    
    // TODO: Get rid of this ugly and dangerous madness:
    Renderer& thisRenderer = *static_cast<Renderer*>(this);

    myCommandAllocatorPool = new CommandAllocatorPoolDX12(thisRenderer, D3D12_COMMAND_LIST_TYPE_DIRECT);
    myDefaultContext = new RenderContext(thisRenderer);
    myDescriptorHeapPool = new DescriptorHeapPoolDX12(*this);
    
    CreateBackbufferResources();
	}
//---------------------------------------------------------------------------//
	RendererDX12::~RendererDX12()
	{
    SAFE_DELETE(myDefaultContext);
    SAFE_DELETE(myCommandAllocatorPool);
    SAFE_DELETE(myDescriptorHeapPool);
	}
//---------------------------------------------------------------------------// 
  uint64 RendererDX12::ExecuteCommandList(ID3D12CommandList* aCommandList)
  {
    myFence.wait();

    myCommandQueue->ExecuteCommandLists(1, &aCommandList);
    return myFence.signal(myCommandQueue.Get());
  }
//---------------------------------------------------------------------------//
  void RendererDX12::CreateDeviceAndSwapChain(void* aNativeWindowHandle)
  {
    using namespace Microsoft::WRL;

    ComPtr<ID3D12Debug> debugInterface;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))))
      debugInterface->EnableDebugLayer();

    CheckD3Dcall(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&myDevice)));

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    CheckD3Dcall(myDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&myCommandQueue)));

    HWND windowHandle = *static_cast<HWND*>(aNativeWindowHandle);

    ComPtr<IDXGIFactory4> dxgiFactory;
    CheckD3Dcall(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = kBackbufferCount;
    swapChainDesc.BufferDesc.Width = 1280;
    swapChainDesc.BufferDesc.Height = 720;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.OutputWindow = windowHandle;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;

    ComPtr<IDXGISwapChain> swapChain;
    CheckD3Dcall(dxgiFactory->CreateSwapChain(myCommandQueue.Get(), &swapChainDesc, &swapChain));
    CheckD3Dcall(swapChain.As(&mySwapChain));
    myCurrBackbufferIndex = mySwapChain->GetCurrentBackBufferIndex();

    // Create synchronization objects.
    myFence.Init(myDevice.Get(), "RendererDX12::FrameDone");
  }
//---------------------------------------------------------------------------//
  void RendererDX12::CreateBackbufferResources()
  {
    DescriptorHeapDX12* rtvHeapCpu = myDescriptorHeapPool->GetStaticHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeapCpu->GetCpuHeapStart();
    
    for (UINT n = 0; n < kBackbufferCount; n++)
    {
      TextureDX12& backbufferResource = myBackbuffers[n];
      backbufferResource.myUsageState = D3D12_RESOURCE_STATE_PRESENT;

      CheckD3Dcall(mySwapChain->GetBuffer(n, IID_PPV_ARGS(&backbufferResource.myResource)));
      myDevice->CreateRenderTargetView(backbufferResource.myResource.Get(), nullptr, rtvHandle);
      backbufferResource.myRtv = rtvHandle;
      
      rtvHandle.ptr += rtvHeapCpu->GetHandleIncrementSize();
    }
  }
//---------------------------------------------------------------------------//
  void RendererDX12::postInit()
	{
    // DEBUG: Compile a shader
    // GpuProgramPermutation permutation;
    // 
    // GpuProgramDesc vertexProgramDesc;
    // vertexProgramDesc.myPermutation = permutation;
    // vertexProgramDesc.myShaderPath = Rendering::GpuProgramCompiler::GetPlatformShaderFileDirectory() +
    //   String("MaterialForward") + Rendering::GpuProgramCompiler::GetPlatformShaderFileExtension();
    // vertexProgramDesc.myShaderStage = static_cast<uint32>(ShaderStage::VERTEX);
    // GpuProgram* pVertexProgram = GpuProgramCompiler::createOrRetrieve(vertexProgramDesc);
	}
//---------------------------------------------------------------------------//
  void RendererDX12::beginFrame()
	{
    myFence.wait();
    myCurrBackbufferIndex = mySwapChain->GetCurrentBackBufferIndex();
	}
//---------------------------------------------------------------------------//
	void RendererDX12::endFrame()
	{
    DescriptorHeapDX12* rtvHeap = myDescriptorHeapPool->GetStaticHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    //myDefaultContext->SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, rtvHeap);

    TextureDX12& currBackbuffer = myBackbuffers[myCurrBackbufferIndex];
    myDefaultContext->TransitionResource(&currBackbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
    
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    myDefaultContext->ClearRenderTargetView(currBackbuffer.GetRTV(), clearColor);

    myDefaultContext->TransitionResource(&currBackbuffer, D3D12_RESOURCE_STATE_PRESENT, true);
    
    myDefaultContext->ExecuteAndReset(false);

    mySwapChain->Present(1, 0);
	}
//---------------------------------------------------------------------------//
  void RendererDX12::WaitForFence(uint64 aFenceVal)
  {
    if (myFence.IsDone(aFenceVal))
      return;

    if (myFence.GetCurrWaitingFenceVal() >= aFenceVal)
    {
      myFence.wait();
    }
    else
    {
      myFence.signal(myCommandQueue.Get(), aFenceVal);
      myFence.wait();
    }
  }
//---------------------------------------------------------------------------//
  void RenderingSubsystemDX12::InitPlatform()
  {
    RootSignaturePoolDX12::Init();
  }
//---------------------------------------------------------------------------//
  void RenderingSubsystemDX12::ShutdownPlatform()
  {
    RootSignaturePoolDX12::Destroy();
  }
//---------------------------------------------------------------------------//
} } }  // end of namespace Fancy::Rendering::DX12

#endif