#pragma once

#include "RenderCore_Platform.h"

#include "Texture.h"
#include "FenceDX12.h"
#include "RenderContextDX12.h"
#include "CommandAllocatorPoolDX12.h"
#include "DescriptorHeapPoolDX12.h"

namespace Fancy {
  class RenderWindow;
}

namespace Fancy { namespace Rendering {
  class ShaderResourceInterface;
  class GeometryVertexLayout;
} }

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//  
  class RenderCore_PlatformDX12 : public RenderCore_Platform
  {
  public:
    RenderCore_PlatformDX12();
    ~RenderCore_PlatformDX12() override;

    bool IsInitialized() override { return ourDevice.Get() != nullptr; }

    static DXGI_FORMAT GetFormat(DataFormat aFormat);
    static DataFormat ResolveFormat(DataFormat aFormat);
    static DataFormatInfo GetFormatInfo(DXGI_FORMAT aFormat);
    static D3D12_COMMAND_LIST_TYPE GetCommandListType(CommandListType aType);

    ID3D12Device* GetDevice() const { return ourDevice.Get(); }

    Rendering::ShaderResourceInterface*
      GetShaderResourceInterface(const D3D12_ROOT_SIGNATURE_DESC& anRSdesc, ComPtr<ID3D12RootSignature> anRS = nullptr) const;

    void WaitForFence(CommandListType aType, uint64 aFenceVal);
    bool IsFenceDone(CommandListType aType, uint64 aFenceVal);

    uint64 ExecuteCommandList(ID3D12CommandList* aCommandList);

    ID3D12CommandAllocator* GetCommandAllocator(CommandListType aCmdListType);
    void ReleaseCommandAllocator(ID3D12CommandAllocator* anAllocator, CommandListType aCmdListType, uint64 aFenceVal);

    DescriptorDX12 AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType);
    DescriptorHeapPoolDX12* GetDescriptorHeapPool() const { return ourDynamicDescriptorHeapPool; }
    DescriptorHeapDX12* AllocateDynamicDescriptorHeap(uint32 aDescriptorCount, D3D12_DESCRIPTOR_HEAP_TYPE aHeapType);
    void ReleaseDynamicDescriptorHeap(DescriptorHeapDX12* aHeap, CommandListType aCmdListType, uint64 aFenceVal);

    GpuProgramCompiler* CreateShaderCompiler() override;
    GpuProgram* CreateGpuProgram() override;
    Texture* CreateTexture() override;
    GpuBuffer* CreateGpuBuffer() override;
    CommandContext* CreateContext(CommandListType aType) override;
    void InitBufferData(GpuBuffer* aBuffer, void* aDataPtr, CommandContext* aContext) override;
    void UpdateBufferData(GpuBuffer* aBuffer, void* aDataPtr, uint32 aByteOffset, uint32 aByteSize, CommandContext* aContext) override;
    void InitTextureData(Texture* aTexture, const TextureUploadData* someUploadDatas, uint32 aNumUploadDatas, CommandContext* aContext) override;


    ComPtr<ID3D12Device> ourDevice;

    CommandAllocatorPoolDX12* ourCommandAllocatorPools[(uint)CommandListType::NUM];
    ComPtr<ID3D12CommandQueue> ourCommandQueues[(uint)CommandListType::NUM];
    FenceDX12 ourCmdListDoneFences[(uint)CommandListType::NUM];

  protected:
    DescriptorHeapPoolDX12* ourDynamicDescriptorHeapPool;
    DescriptorHeapDX12 ourStaticDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
  };
//---------------------------------------------------------------------------//
} } }
