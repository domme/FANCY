#pragma once

#include "RenderCore_Platform.h"

#include "DX12Prerequisites.h"
#include "Texture.h"
#include "FenceDX12.h"
#include "CommandAllocatorPoolDX12.h"
#include "DescriptorHeapDX12.h"
#include "DescriptorDX12.h"

#include <queue>

namespace Fancy {
//---------------------------------------------------------------------------//
  class Window;
  class ShaderResourceInterface;
  class GeometryVertexLayout;
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
    
    static DXGI_FORMAT GetFormat(DataFormat aFormat);
    static DataFormatInfo GetFormatInfo(DXGI_FORMAT aFormat);
    static D3D12_COMMAND_LIST_TYPE GetCommandListType(CommandListType aType);
    static DXGI_FORMAT GetDepthStencilFormat(DXGI_FORMAT aDefaultFormat);
    static DXGI_FORMAT GetDepthFormat(DXGI_FORMAT aDefaultFormat);
    static DXGI_FORMAT GetStencilFormat(DXGI_FORMAT aDefaultFormat);

    ID3D12Device* GetDevice() const { return ourDevice.Get(); }

    ShaderResourceInterface*
      GetShaderResourceInterface(const D3D12_ROOT_SIGNATURE_DESC& anRSdesc, Microsoft::WRL::ComPtr<ID3D12RootSignature> anRS = nullptr) const;

    void WaitForFence(CommandListType aType);
    void WaitForFence(CommandListType aType, uint64 aFenceVal);
    bool IsFenceDone(CommandListType aType, uint64 aFenceVal);

    uint64 ExecuteCommandList(ID3D12CommandList* aCommandList);

    ID3D12CommandAllocator* GetCommandAllocator(CommandListType aCmdListType);
    void ReleaseCommandAllocator(ID3D12CommandAllocator* anAllocator, CommandListType aCmdListType, uint64 aFenceVal);

    DescriptorDX12 AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType);
    DescriptorHeapDX12* AllocateDynamicDescriptorHeap(uint aDescriptorCount, D3D12_DESCRIPTOR_HEAP_TYPE aHeapType);
    void ReleaseDynamicDescriptorHeap(DescriptorHeapDX12* aHeap, CommandListType aCmdListType, uint64 aFenceVal);

    RenderOutput* CreateRenderOutput(void* aNativeInstanceHandle) override;
    GpuProgramCompiler* CreateShaderCompiler() override;
    GpuProgram* CreateGpuProgram() override;
    Texture* CreateTexture() override;
    GpuBuffer* CreateGpuBuffer() override;
    CommandContext* CreateContext(CommandListType aType) override;
    void InitBufferData(GpuBuffer* aBuffer, void* aDataPtr, CommandContext* aContext) override;
    void UpdateBufferData(GpuBuffer* aBuffer, void* aDataPtr, uint aByteOffset, uint aByteSize, CommandContext* aContext) override;
    void InitTextureData(Texture* aTexture, const TextureUploadData* someUploadDatas, uint aNumUploadDatas, CommandContext* aContext) override;
    DataFormat ResolveFormat(DataFormat aFormat) override;

    // TODO: Make this more platform-independent if we need a platform-independent swap-chain representation (how does Vulkan handle it?)
    Microsoft::WRL::ComPtr<IDXGISwapChain> CreateSwapChain(const DXGI_SWAP_CHAIN_DESC& aSwapChainDesc);

    Microsoft::WRL::ComPtr<ID3D12Device> ourDevice;

    CommandAllocatorPoolDX12* ourCommandAllocatorPools[(uint)CommandListType::NUM];
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> ourCommandQueues[(uint)CommandListType::NUM];
    FenceDX12 ourCmdListDoneFences[(uint)CommandListType::NUM];

  protected:
    struct FenceInfo
    {
      CommandListType myType;
      uint64 myFenceVal;
    };

    std::vector<std::unique_ptr<DescriptorHeapDX12>> myDynamicHeapPool;
    std::deque<DescriptorHeapDX12*> myAvailableDynamicHeaps;
    std::queue<std::pair<FenceInfo, DescriptorHeapDX12*>> myUsedDynamicHeaps;
    
    DescriptorHeapDX12 ourStaticDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    void InitCaps() override;
  };
//---------------------------------------------------------------------------//
}
