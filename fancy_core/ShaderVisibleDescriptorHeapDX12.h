#pragma once

#include "FancyCoreDefines.h"
#include "DX12Prerequisites.h"
#include "DescriptorDX12.h"
#include "RenderCore.h"
#include "PagedLinearAllocator.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
//---------------------------------------------------------------------------//
  // Wraps a shader-visible descriptor heap and manages descriptor-allocations.
//---------------------------------------------------------------------------//
  class ShaderVisibleDescriptorHeapDX12
  {
    friend class RenderCore_PlatformDX12;

  public:
    ShaderVisibleDescriptorHeapDX12(const RenderPlatformProperties& someProperties);
    
    ID3D12DescriptorHeap* GetResourceHeap() const { return myResourceHeap.Get(); }
    ID3D12DescriptorHeap* GetSamplerHeap() const { return mySamplerHeap.Get(); }

    DescriptorDX12 AllocateDescriptor(GlobalResourceType aType, const char* aDebugName = nullptr);
    void FreeDescriptorAfterFrameDone(const DescriptorDX12& aDescriptor);

    D3D12_GPU_DESCRIPTOR_HANDLE GetHeapStart(GlobalResourceType aType) const { return myGlobalDescriptorGpuHeapStart[aType]; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetResourceHeapStart() const { return myResourceHeap->GetGPUDescriptorHandleForHeapStart(); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetSamplerHeapStart() const { return mySamplerHeap->GetGPUDescriptorHandleForHeapStart(); }

  private:
    void ProcessGlobalDescriptorFrees();
    D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType(GlobalResourceType aResourceType) const { return aResourceType == GLOBAL_RESOURCE_SAMPLER ? D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER : D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; }
    uint GetHandleIncementSize(GlobalResourceType aResourceType) const { return aResourceType == GLOBAL_RESOURCE_SAMPLER ? mySamplerHandleIncrementSize : myResourceHandleIncrementSize; }

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> myResourceHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mySamplerHeap;

    uint myNumGlobalDescriptors[GLOBAL_RESOURCE_NUM];

    uint myResourceHandleIncrementSize;
    uint mySamplerHandleIncrementSize;

    D3D12_CPU_DESCRIPTOR_HANDLE myGlobalDescriptorCpuHeapStart[GLOBAL_RESOURCE_NUM];
    D3D12_GPU_DESCRIPTOR_HANDLE myGlobalDescriptorGpuHeapStart[GLOBAL_RESOURCE_NUM];

    PagedLinearAllocator myAllocators[GLOBAL_RESOURCE_NUM];

    struct DescriptorToFree
    {
      GlobalResourceType myType;
      uint myIndex;
    };

    static constexpr uint ourNumGlobalFreeLists = RenderCore::Constants::NUM_QUEUED_FRAMES + 1;
    eastl::fixed_vector<DescriptorToFree, 256> myDescriptorsToFree[ourNumGlobalFreeLists];
  };
//---------------------------------------------------------------------------// 
}

#endif