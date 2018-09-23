#pragma once

#include "RenderCore_Platform.h"

#include "DX12Prerequisites.h"
#include "Texture.h"
#include "CommandAllocatorPoolDX12.h"
#include "DynamicDescriptorHeapDX12.h"
#include "DescriptorDX12.h"

#include <queue>
#include "CommandQueueDX12.h"
#include "GpuMemoryAllocatorDX12.h"
#include "GpuBuffer.h"
#include "StaticDescriptorAllocatorDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class Window;
  class ShaderResourceInterface;
  class GeometryVertexLayout;
  struct GpuResourceViewData;
  struct TextureViewProperties;
//---------------------------------------------------------------------------//  
  class RenderCore_PlatformDX12 final : public RenderCore_Platform
  {
  public:
    RenderCore_PlatformDX12();
    ~RenderCore_PlatformDX12() override;
    // Disallow copy and assignment (class contains a list of unique_ptrs)
    RenderCore_PlatformDX12(const RenderCore_PlatformDX12&) = delete;
    RenderCore_PlatformDX12& operator=(const RenderCore_PlatformDX12&) = delete;

    bool IsInitialized() override { return ourDevice.Get() != nullptr; }
    bool InitInternalResources() override;
    
    static DXGI_FORMAT GetDXGIformat(DataFormat aFormat);
    static DXGI_FORMAT GetDepthStencilTextureFormat(DXGI_FORMAT aFormat);
    static DXGI_FORMAT GetDepthStencilViewFormat(DXGI_FORMAT aFormat);
    static DXGI_FORMAT GetDepthViewFormat(DXGI_FORMAT aFormat);
    static DXGI_FORMAT GetStencilViewFormat(DXGI_FORMAT aFormat);
    static DXGI_FORMAT GetTypelessFormat(DXGI_FORMAT aFormat);

    static D3D12_COMMAND_LIST_TYPE GetCommandListType(CommandListType aType);
    static D3D12_HEAP_TYPE ResolveHeapType(GpuMemoryAccessType anAccessType);

    ID3D12Device* GetDevice() const { return ourDevice.Get(); }

    ShaderResourceInterface*
      GetShaderResourceInterface(const D3D12_ROOT_SIGNATURE_DESC& anRSdesc, Microsoft::WRL::ComPtr<ID3D12RootSignature> anRS = nullptr) const;

    ID3D12CommandAllocator* GetCommandAllocator(CommandListType aCmdListType);
    void ReleaseCommandAllocator(ID3D12CommandAllocator* anAllocator, uint64 aFenceVal);

    DescriptorDX12 AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType);
    void ReleaseDescriptor(const DescriptorDX12& aDescriptor);

    DynamicDescriptorHeapDX12* AllocateDynamicDescriptorHeap(uint aDescriptorCount, D3D12_DESCRIPTOR_HEAP_TYPE aHeapType);
    void ReleaseDynamicDescriptorHeap(DynamicDescriptorHeapDX12* aHeap, uint64 aFenceVal);

    GpuMemoryAllocationDX12 AllocateGpuMemory(GpuMemoryType aType, GpuMemoryAccessType anAccessType, uint64 aSize, uint anAlignment);
    void ReleaseGpuMemory(GpuMemoryAllocationDX12& anAllocation);

    RenderOutput* CreateRenderOutput(void* aNativeInstanceHandle) override;
    GpuProgramCompiler* CreateShaderCompiler() override;
    GpuProgram* CreateGpuProgram() override;
    Texture* CreateTexture() override;
    GpuBuffer* CreateBuffer() override;
    CommandContext* CreateContext(CommandListType aType) override;
    DataFormat ResolveFormat(DataFormat aFormat) override;
    CommandQueue* GetCommandQueue(CommandListType aType) override { return ourCommandQueues[(uint)aType].get(); }
    TextureView* CreateTextureView(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties) override;
    GpuBufferView* CreateBufferView(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties) override;

    // TODO: Make this more platform-independent if we need a platform-independent swap-chain representation (how does Vulkan handle it?)
    Microsoft::WRL::ComPtr<IDXGISwapChain> CreateSwapChain(const DXGI_SWAP_CHAIN_DESC& aSwapChainDesc);

    Microsoft::WRL::ComPtr<ID3D12Device> ourDevice;

    CommandAllocatorPoolDX12* ourCommandAllocatorPools[(uint)CommandListType::NUM];
	  std::unique_ptr<CommandQueueDX12> ourCommandQueues[(uint)CommandListType::NUM];

  protected:
    void InitCaps() override;
    DescriptorDX12 CreateSRV(const Texture* aTexture, const TextureViewProperties& someProperties);
    DescriptorDX12 CreateSRV(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someProperties);

    DescriptorDX12 CreateUAV(const Texture* aTexture, const TextureViewProperties& someProperties);
    DescriptorDX12 CreateUAV(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someProperties);

    DescriptorDX12 CreateRTV(const Texture* aTexture, const TextureViewProperties& someProperties);
    
    DescriptorDX12 CreateDSV(const Texture* aTexture, const TextureViewProperties& someProperties);

    DescriptorDX12 CreateCBV(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someProperties);
    
    std::vector<std::unique_ptr<DynamicDescriptorHeapDX12>> myDynamicHeapPool;
    std::list<DynamicDescriptorHeapDX12*> myAvailableDynamicHeaps;
    std::list<std::pair<uint64, DynamicDescriptorHeapDX12*>> myUsedDynamicHeaps;

    UniquePtr<StaticDescriptorAllocatorDX12> myStaticDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    UniquePtr<GpuMemoryAllocatorDX12> myGpuMemoryAllocators[(uint)GpuMemoryType::NUM][(uint)GpuMemoryAccessType::NUM];
  };
//---------------------------------------------------------------------------//
}
