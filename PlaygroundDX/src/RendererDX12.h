#pragma once

#include "Prerequisites.h"

class RendererDX12
{
public:
	virtual ~RendererDX12();

	void init(uint aWidth, uint aHeight, void* aNativeWindowHandle);
	void postInit();

	void beginFrame();
	void render();
	void endFrame();

	static const uint kFrameCount = 2u;
	Microsoft::WRL::ComPtr<ID3D12Device> myDevice;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> mySwapChain;
	Microsoft::WRL::ComPtr<ID3D12Resource> myRenderTargets[kFrameCount];
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> myCommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> myCommandQueue;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> myRtvHeap;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> myPipelineState;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> myCommandList;
	uint myRtvDescriptorSize;

	// Synchronization objects.
	uint myFrameIndex;
	HANDLE myFenceEvent;
	Microsoft::WRL::ComPtr<ID3D12Fence> myFence;
	uint myFenceValue;
};