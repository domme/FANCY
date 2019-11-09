#pragma once

#include "RenderCore_Platform.h"
#include "DX12Prerequisites.h"

#include "DataFormat.h"
#include "Ptr.h"
#include "DynamicDescriptorHeapDX12.h"
#include "StaticDescriptorAllocatorDX12.h"
#include "GpuMemoryAllocatorDX12.h"
#include "CommandAllocatorPoolDX12.h"
#include "CommandQueueDX12.h"
#include "GpuQueryHeap.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct WindowParameters;
  class Shader;
  class Texture;
  class TextureView;
  struct TextureViewProperties;
  class GpuBuffer;
  class GpuBufferView;
  struct GpuBufferViewProperties;
//---------------------------------------------------------------------------//  
  class RenderCore_PlatformDX12 final : public RenderCore_Platform
  {
  public:
    static D3D12_LOGIC_OP ResolveLogicOp(LogicOp aLogicOp);
    static D3D12_COMPARISON_FUNC ResolveCompFunc(const CompFunc& aCompFunc);
    static D3D12_STENCIL_OP ResolveStencilOp(const StencilOp& aStencilOp);
    static DXGI_FORMAT ResolveFormat(DataFormat aFormat);
    static DXGI_FORMAT GetDepthStencilTextureFormat(DXGI_FORMAT aFormat);
    static DXGI_FORMAT GetDepthStencilViewFormat(DXGI_FORMAT aFormat);
    static DXGI_FORMAT GetDepthViewFormat(DXGI_FORMAT aFormat);
    static DXGI_FORMAT GetStencilViewFormat(DXGI_FORMAT aFormat);
    static DXGI_FORMAT GetTypelessFormat(DXGI_FORMAT aFormat);
    static D3D12_COMMAND_LIST_TYPE GetCommandListType(CommandListType aType);
    static D3D12_HEAP_TYPE ResolveHeapType(CpuMemoryAccessType anAccessType);
    static D3D12_RESOURCE_STATES ResolveResourceUsageState(GpuResourceState aState);

    RenderCore_PlatformDX12();
    ~RenderCore_PlatformDX12() override;
    // Disallow copy and assignment (class contains a list of unique_ptrs)
    RenderCore_PlatformDX12(const RenderCore_PlatformDX12&) = delete;
    RenderCore_PlatformDX12& operator=(const RenderCore_PlatformDX12&) = delete;

    bool IsInitialized() override { return ourDevice.Get() != nullptr; }
    bool InitInternalResources() override;
    void Shutdown() override;

    ID3D12Device* GetDevice() const { return ourDevice.Get(); }

    ID3D12CommandAllocator* GetCommandAllocator(CommandListType aCmdListType);
    void ReleaseCommandAllocator(ID3D12CommandAllocator* anAllocator, uint64 aFenceVal);

    DescriptorDX12 AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType, const char* aDebugName = nullptr);
    void ReleaseDescriptor(const DescriptorDX12& aDescriptor);

    DynamicDescriptorHeapDX12* AllocateDynamicDescriptorHeap(uint aDescriptorCount, D3D12_DESCRIPTOR_HEAP_TYPE aHeapType);
    void ReleaseDynamicDescriptorHeap(DynamicDescriptorHeapDX12* aHeap, uint64 aFenceVal);

    GpuMemoryAllocationDX12 AllocateGpuMemory(GpuMemoryType aType, CpuMemoryAccessType anAccessType, uint64 aSize, uint anAlignment, const char* aDebugName = nullptr);
    void ReleaseGpuMemory(GpuMemoryAllocationDX12& anAllocation);

    RenderOutput* CreateRenderOutput(void* aNativeInstanceHandle, const WindowParameters& someWindowParams) override;
    ShaderCompiler* CreateShaderCompiler() override;
    Shader* CreateShader() override;
    ShaderPipeline* CreateShaderPipeline() override;
    Texture* CreateTexture() override;
    GpuBuffer* CreateBuffer() override;
    CommandList* CreateCommandList(CommandListType aType) override;
    CommandQueue* CreateCommandQueue(CommandListType aType) override;
    TextureView* CreateTextureView(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties, const char* aDebugName = nullptr) override;
    GpuBufferView* CreateBufferView(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties, const char* aDebugName = nullptr) override;
    GpuQueryHeap* CreateQueryHeap(GpuQueryType aType, uint aNumQueries) override;
    uint GetQueryTypeDataSize(GpuQueryType aType) override;
    float64 GetGpuTicksToMsFactor(CommandListType aCommandListType) override;
    
    Microsoft::WRL::ComPtr<IDXGISwapChain> CreateSwapChain(const DXGI_SWAP_CHAIN_DESC& aSwapChainDesc);

  // protected:
    void UpdateAvailableDynamicDescriptorHeaps();
    CommandQueueDX12* GetCommandQueueDX12(CommandListType aCommandListType);

    Microsoft::WRL::ComPtr<ID3D12Device> ourDevice;

    // TODO: Move the dynamic heaps to a dedicated pool-class? (Similar to ComandAllocatorPoolDX12)
    std::vector<UniquePtr<DynamicDescriptorHeapDX12>> myDynamicHeapPool;
    std::list<DynamicDescriptorHeapDX12*> myAvailableDynamicHeaps;
    std::list<std::pair<uint64, DynamicDescriptorHeapDX12*>> myUsedDynamicHeaps;

    UniquePtr<StaticDescriptorAllocatorDX12> myStaticDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    UniquePtr<GpuMemoryAllocatorDX12> myGpuMemoryAllocators[(uint)GpuMemoryType::NUM][(uint)CpuMemoryAccessType::NUM];
    UniquePtr<CommandAllocatorPoolDX12> ourCommandAllocatorPools[(uint)CommandListType::NUM];
    float64 myGpuTicksToMsFactor[(uint)CommandListType::NUM];

  private:
    static bool CreateSRV(const Texture* aTexture, const TextureViewProperties& someProperties, const DescriptorDX12& aDescriptor);
    static bool CreateUAV(const Texture* aTexture, const TextureViewProperties& someProperties, const DescriptorDX12& aDescriptor);
    static bool CreateRTV(const Texture* aTexture, const TextureViewProperties& someProperties, const DescriptorDX12& aDescriptor);
    static bool CreateDSV(const Texture* aTexture, const TextureViewProperties& someProperties, const DescriptorDX12& aDescriptor);
  };
//---------------------------------------------------------------------------//
}
