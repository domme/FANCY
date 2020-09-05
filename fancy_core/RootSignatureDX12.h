#pragma once

#include "DX12Prerequisites.h"
#include "ShaderResourceInfoDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct RootSignatureLayoutDX12
  {
    RootSignatureLayoutDX12() = default;
    RootSignatureLayoutDX12(const D3D12_ROOT_SIGNATURE_DESC1& aRootSigDesc);

    uint64 GetHash() const;

    struct DescriptorTable
    {
      eastl::fixed_vector<D3D12_DESCRIPTOR_RANGE1, 8> myRanges;
    };

    struct RootParameter
    {
      D3D12_ROOT_PARAMETER_TYPE myType;
      D3D12_SHADER_VISIBILITY myVisiblity;
      DescriptorTable myDescriptorTable;
      D3D12_ROOT_CONSTANTS myRootConstants;
      D3D12_ROOT_DESCRIPTOR1 myRootDescriptor;
    };

    eastl::fixed_vector<RootParameter, 8> myRootParameters;
  };
//---------------------------------------------------------------------------//
  struct RootSignatureBindingsDX12
  {
    RootSignatureBindingsDX12(const RootSignatureLayoutDX12& aLayout);

    struct DescriptorRange
    {
      bool myIsUnbounded = false;
      D3D12_DESCRIPTOR_RANGE_TYPE myType;
      eastl::fixed_vector<D3D12_CPU_DESCRIPTOR_HANDLE, 8> myDescriptors;
    };

    struct DescriptorTable
    {
      D3D12_DESCRIPTOR_HEAP_TYPE myHeapType;
      mutable bool myIsDirty = true;
      bool myHasUnboundedRanges = false;
      uint myBoundedNumDescriptors = 0u;  // Only valid if none of the descriptor ranges is unbounded. 
      eastl::fixed_vector<DescriptorRange, 8> myRanges;
    };

    struct RootDescriptor
    {
      ShaderResourceTypeDX12 myType = ShaderResourceTypeDX12::None;
      uint64 myGpuVirtualAddress = 0ull;
      mutable bool myIsDirty = true;
    };

    struct RootParameter
    {
      bool myIsDescriptorTable = false;
      RootDescriptor myRootDescriptor;
      DescriptorTable myDescriptorTable;
    };

    eastl::fixed_vector<RootParameter, 8> myRootParameters;
    mutable bool myIsDirty = true;

    void Clear();
  };
//---------------------------------------------------------------------------//
}

#endif