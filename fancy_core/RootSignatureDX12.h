#pragma once

#include "DX12Prerequisites.h"
#include "ShaderResourceInfoDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct RootSignatureBindingsDX12
  {
    struct DescriptorRange
    {
      D3D12_DESCRIPTOR_RANGE_TYPE myType;
      StaticArray<D3D12_CPU_DESCRIPTOR_HANDLE, 64> myDescriptors;
    };

    struct DescriptorTable
    {
      D3D12_DESCRIPTOR_HEAP_TYPE myHeapType;
      uint myNumDescriptors;
      StaticArray<DescriptorRange, 64> myRanges;
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

    StaticArray<RootParameter, 64> myRootParameters;

    void Clear();
  };
//---------------------------------------------------------------------------//
  struct RootSignatureLayoutDX12
  {
    RootSignatureLayoutDX12() = default;
    RootSignatureLayoutDX12(const D3D12_ROOT_SIGNATURE_DESC1& aRootSigDesc);

    RootSignatureBindingsDX12 Instantiate() const;

    struct DescriptorTable
    {
      StaticArray<D3D12_DESCRIPTOR_RANGE1, 64> myRanges;
    };

    struct RootParameter
    {
      D3D12_ROOT_PARAMETER_TYPE myType;
      D3D12_SHADER_VISIBILITY myVisiblity;
      DescriptorTable myDescriptorTable;
      D3D12_ROOT_CONSTANTS myRootConstants;
      D3D12_ROOT_DESCRIPTOR1 myRootDescriptor;
    };

    StaticArray<RootParameter, 64> myRootParameters;
  };
//---------------------------------------------------------------------------//
}

#endif