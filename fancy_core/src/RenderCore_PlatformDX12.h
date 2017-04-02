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

    bool IsInitialized() { return ourDevice.Get() != nullptr; }

    DXGI_FORMAT GetFormat(DataFormat aFormat);
    DataFormat ResolveFormat(DataFormat aFormat);
    DataFormatInfo GetFormatInfo(DXGI_FORMAT aFormat);
    D3D12_COMMAND_LIST_TYPE GetCommandListType(CommandListType aType);

    ID3D12Device* GetDevice() { return ourDevice.Get(); }

    Rendering::ShaderResourceInterface*
      GetShaderResourceInterface(const D3D12_ROOT_SIGNATURE_DESC& anRSdesc, ComPtr<ID3D12RootSignature> anRS = nullptr);

    void WaitForFence(CommandListType aType, uint64 aFenceVal);
    bool IsFenceDone(CommandListType aType, uint64 aFenceVal);

    uint64 ExecuteCommandList(ID3D12CommandList* aCommandList);

    ID3D12CommandAllocator* GetCommandAllocator(CommandListType aCmdListType);
    void ReleaseCommandAllocator(ID3D12CommandAllocator* anAllocator, CommandListType aCmdListType, uint64 aFenceVal);

    Descriptor AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType);
    DescriptorHeapPoolDX12* GetDescriptorHeapPool() { return ourDynamicDescriptorHeapPool; }
    DescriptorHeapDX12* AllocateDynamicDescriptorHeap(uint32 aDescriptorCount, D3D12_DESCRIPTOR_HEAP_TYPE aHeapType);
    void ReleaseDynamicDescriptorHeap(DescriptorHeapDX12* aHeap, CommandListType aCmdListType, uint64 aFenceVal);

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
