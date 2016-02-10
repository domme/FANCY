#include "RendererDX12.h"
#include "AdapterDX12.h"

#if defined (RENDERER_DX12)
#include "MathUtil.h"
#include "GpuProgram.h"
#include "RootSignatureDX12.h"
#include "GpuProgramCompiler.h"
#include "Renderer.h"

namespace Fancy { namespace Rendering { namespace DX12 { 

//---------------------------------------------------------------------------//
  RendererDX12::RendererDX12(void* aNativeWindowHandle)
    : myCommandAllocatorPool(nullptr)
    , myDefaultContext(nullptr)
	{
    CreateDeviceAndSwapChain(aNativeWindowHandle);
    
    myCommandAllocatorPool = new CommandAllocatorPoolDX12(*static_cast<Renderer*>(this), D3D12_COMMAND_LIST_TYPE_DIRECT);
    myDefaultContext = new RenderContext(*static_cast<Renderer*>(this));

    CreateBackbufferResources();
	}
//---------------------------------------------------------------------------//
	RendererDX12::~RendererDX12()
	{
    SAFE_DELETE(myDefaultContext);
    SAFE_DELETE(myCommandAllocatorPool);
	}
//---------------------------------------------------------------------------//
  uint64 RendererDX12::ExecuteCommandList(ID3D12CommandList* aCommandList)
  {
    mySyncFence.wait();

    myCommandQueue->ExecuteCommandLists(1, &aCommandList);
    return mySyncFence.signal(myCommandQueue.Get());
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
  }
//---------------------------------------------------------------------------//
  void RendererDX12::CreateBackbufferResources()
  {
    ID3D12DescriptorHeap* rtvHeap;
    uint32 rtvHeapIncrSize;

    // Create descriptor heaps.
    // Describe and create a render target view (RTV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = kBackbufferCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    CheckD3Dcall(myDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)));

    rtvHeapIncrSize = myDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // Create frame resources.
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
    uint rtvHandleOffset = 0;

    // Create a RTV for each backbuffer.

    for (UINT n = 0; n < kBackbufferCount; n++)
    {
      mySwapChain->GetBuffer(n, IID_PPV_ARGS(&myBackbuffers[n]));
      myDevice->CreateRenderTargetView(myBackbuffers[n].Get(), nullptr, rtvHandle);
      rtvHandle.ptr += rtvHeapIncrSize;
    }

    // Setup default renderContext
    myDefaultContext->SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, rtvHeap);
  }
//---------------------------------------------------------------------------//
  void RendererDX12::postInit()
	{
    // Create synchronization objects.
    mySyncFence.init(myDevice.Get(), "RendererDX12::FrameDone");

    // DEBUG: Compile a shader
    GpuProgramPermutation permutation;

    GpuProgramDesc vertexProgramDesc;
    vertexProgramDesc.myPermutation = permutation;
    vertexProgramDesc.myShaderPath = Rendering::GpuProgramCompiler::GetPlatformShaderFileDirectory() +
      String("MaterialForward") + Rendering::GpuProgramCompiler::GetPlatformShaderFileExtension();
    vertexProgramDesc.myShaderStage = static_cast<uint32>(ShaderStage::VERTEX);
    GpuProgram* pVertexProgram = GpuProgramCompiler::createOrRetrieve(vertexProgramDesc);
	}
//---------------------------------------------------------------------------//
  void RendererDX12::beginFrame()
	{
    ComPtr<ID3D12GraphicsCommandList>&  cmdList = getGraphicsCmdList();

    myFrameDone.wait();

    myCommandAllocator->Reset();
    cmdList->Reset(myCommandAllocator.Get(), nullptr);
    myCurrBackbufferIndex = mySwapChain->GetCurrentBackBufferIndex();
	}
//---------------------------------------------------------------------------//
	void RendererDX12::endFrame()
	{
    ComPtr<ID3D12GraphicsCommandList>&  cmdList = getGraphicsCmdList();

    // TODO: Adapt this to multithreaded rendering: Wait until all cmd-lists are completed, patch them with transitional resource-barriers and execute them in-order

    // Move this part to rendering
    ////////////////////////////////////////////////////////////////////////////////
    D3D12_RESOURCE_BARRIER bbBarrier;
    bbBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    bbBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    bbBarrier.Transition.pResource = myBackbuffers[myCurrBackbufferIndex].Get();
    bbBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    bbBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    bbBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    cmdList->ResourceBarrier(1, &bbBarrier);

    D3D12_CPU_DESCRIPTOR_HANDLE backbufferHandle = myRtvHeap->GetCPUDescriptorHandleForHeapStart();
    backbufferHandle.ptr += myCurrBackbufferIndex * myRtvDescriptorSize;

    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    cmdList->ClearRenderTargetView(backbufferHandle, clearColor, 0, nullptr);

    bbBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    bbBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    cmdList->ResourceBarrier(1, &bbBarrier);
    
    cmdList->Close();
    ////////////////////////////////////////////////////////////////////////////////

    ID3D12CommandList* commandLists[] = { cmdList.Get() };
    myCommandQueue->ExecuteCommandLists(1, commandLists);
    mySwapChain->Present(1, 0);

    myFrameDone.signal(myCommandQueue.Get());
	}
//---------------------------------------------------------------------------//

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