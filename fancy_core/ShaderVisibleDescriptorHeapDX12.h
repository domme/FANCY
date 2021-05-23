#pragma once

#include "FancyCoreDefines.h"
#include "DX12Prerequisites.h"
#include "DescriptorDX12.h"
#include "RenderCore.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
//---------------------------------------------------------------------------//
  // Wraps a shader-visible descriptor heap and manages descriptor-allocations.
  // The heap is split into these sections:
  // [      Global Descriptors              ]
  // [Textures|RWTextures|Buffers|RWBuffers ]
  //---------------------------------------------------------------------------//
  class ShaderVisibleDescriptorHeapDX12
  {
    friend class RenderCore_PlatformDX12;

  public:
    ShaderVisibleDescriptorHeapDX12(uint aNumGlobalTextures, uint aNumGlobalRWTextures, uint aNumGlobalBuffers, uint aNumGlobalRWBuffers);
    ShaderVisibleDescriptorHeapDX12(uint aNumGlobalSamplers);

    const D3D12_DESCRIPTOR_HEAP_DESC& GetDesc() const { return myDesc; }
    uint GetHandleIncrementSize() const { return myHandleIncrementSize; }
    ID3D12DescriptorHeap* GetHeap() const { return myDescriptorHeap.Get(); }

    DescriptorDX12 AllocateGlobalDescriptor(GlobalResourceType aType, const char* aDebugName = nullptr);
    void FreeGlobalDescriptorAfterFrameDone(const DescriptorDX12& aDescriptor);

    D3D12_GPU_DESCRIPTOR_HANDLE GetGlobalHeapStart(GlobalResourceType aType) const { return myGlobalDescriptorGpuHeapStart[aType]; }

  private:
    void Init(D3D12_DESCRIPTOR_HEAP_TYPE aType);
    void ProcessGlobalDescriptorFrees();

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> myDescriptorHeap;
    D3D12_DESCRIPTOR_HEAP_DESC myDesc;

    uint myOverallNumGlobalDescriptors;
    uint myNumGlobalDescriptors[GLOBAL_RESOURCE_NUM];

    uint myHandleIncrementSize;

    D3D12_CPU_DESCRIPTOR_HANDLE myCpuHeapStart;
    D3D12_GPU_DESCRIPTOR_HANDLE myGpuHeapStart;
    D3D12_CPU_DESCRIPTOR_HANDLE myGlobalDescriptorCpuHeapStart[GLOBAL_RESOURCE_NUM];
    D3D12_GPU_DESCRIPTOR_HANDLE myGlobalDescriptorGpuHeapStart[GLOBAL_RESOURCE_NUM];

    PagedLinearAllocator myGlobalAllocators[GLOBAL_RESOURCE_NUM];

    static constexpr uint ourNumGlobalFreeLists = RenderCore::Constants::NUM_QUEUED_FRAMES + 1;
    eastl::vector<uint> myGlobalDescriptorsToFree[ourNumGlobalFreeLists][GLOBAL_RESOURCE_NUM];
  };
//---------------------------------------------------------------------------// 
}

#endif