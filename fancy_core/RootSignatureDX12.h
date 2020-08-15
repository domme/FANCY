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
      D3D12_DESCRIPTOR_RANGE_TYPE myType;
      eastl::fixed_vector<D3D12_CPU_DESCRIPTOR_HANDLE, 8> myDescriptors;
    };

    struct DescriptorTable
    {
      D3D12_DESCRIPTOR_HEAP_TYPE myHeapType;
      uint myNumDescriptors;
      eastl::fixed_vector<DescriptorRange, 8> myRanges;
    };

    struct RootDescriptor
    {
      ShaderResourceTypeDX12 myType = ShaderResourceTypeDX12::None;
      uint64 myGpuVirtualAddress = 0ull;
    };

    struct RootParameter
    {
      bool myIsDescriptorTable = false;
      RootDescriptor myRootDescriptor;
      DescriptorTable myDescriptorTable;
    };

    eastl::fixed_vector<RootParameter, 8> myRootParameters;

    void Clear();
  };
//---------------------------------------------------------------------------//
}

#endif