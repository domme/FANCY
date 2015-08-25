#include "RendererDX12.h"

using namespace Microsoft::WRL;

RendererDX12::~RendererDX12()
{

}

void RendererDX12::init(uint aWidth, uint aHeight, void* aNativeWindowHandle)
{
	HRESULT success = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&myDevice));

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	success = myDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&myCommandQueue));

	HWND windowHandle = *static_cast<HWND*>(aNativeWindowHandle);

	ComPtr<IDXGIFactory4> dxgiFactory;
	success = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = kFrameCount;
	swapChainDesc.BufferDesc.Width = aWidth;
	swapChainDesc.BufferDesc.Height = aHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.OutputWindow = windowHandle;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = TRUE;

	ComPtr<IDXGISwapChain> swapChain;
	success = dxgiFactory->CreateSwapChain(myCommandQueue.Get(), &swapChainDesc, &swapChain);
	success = swapChain.As(&mySwapChain);
	myFrameIndex = mySwapChain->GetCurrentBackBufferIndex();

	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = kFrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		success = myDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&myRtvHeap));

		myRtvDescriptorSize = myDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	// Create frame resources.
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(myRtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV for each frame.
		for (UINT n = 0; n < kFrameCount; n++)
		{
			mySwapChain->GetBuffer(n, IID_PPV_ARGS(&myRenderTargets[n]));
			myDevice->CreateRenderTargetView(myRenderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, myRtvDescriptorSize);
		}
	}

	myDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&myCommandAllocator));
}

void RendererDX12::postInit()
{
	myDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, myCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&myCommandList));
	myCommandList->Close();

	// Create synchronization objects.
	{
		myDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&myFence));
		myFenceValue = 1;

		// Create an event handle to use for frame synchronization.
		myFenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
		if (myFenceEvent == nullptr)
		{
			HRESULT_FROM_WIN32(GetLastError());
		}
	}
}

void RendererDX12::beginFrame()
{
	const uint fence = myFenceValue;
	myCommandQueue->Signal(myFence.Get(), fence);
	++myFenceValue;

	if (myFence->GetCompletedValue() < fence)
	{
		myFence->SetEventOnCompletion(fence, myFenceEvent);
		WaitForSingleObject(myFenceEvent, INFINITE);
	}

	myFrameIndex = mySwapChain->GetCurrentBackBufferIndex();
}

void RendererDX12::render()
{
	myCommandAllocator->Reset();

	myCommandList->Reset(myCommandAllocator.Get(), myPipelineState.Get());

	D3D12_RESOURCE_BARRIER rtBarrier;
	rtBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	rtBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	rtBarrier.Transition.pResource = myRenderTargets[myFrameIndex].Get();
	rtBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	rtBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	rtBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	myCommandList->ResourceBarrier(1, &rtBarrier);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(myRtvHeap->GetCPUDescriptorHandleForHeapStart(), myFrameIndex, myRtvDescriptorSize);

	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	myCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	rtBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	rtBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	myCommandList->ResourceBarrier(1, &rtBarrier);

	myCommandList->Close();
}


void RendererDX12::endFrame()
{
	ID3D12CommandList* commandLists[] = { myCommandList.Get() };
	myCommandQueue->ExecuteCommandLists(1, commandLists);

	mySwapChain->Present(1, 0);
}