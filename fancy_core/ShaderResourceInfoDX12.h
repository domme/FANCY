#pragma once

#include "DX12Prerequisites.h"

namespace Fancy
{
//---------------------------------------------------------------------------// 
  enum class ShaderResourceTypeDX12
  {
    None, CBV, SRV, UAV, Sampler
  };
//---------------------------------------------------------------------------// 
  struct ShaderResourceInfoDX12
  {
    bool operator==(const ShaderResourceInfoDX12& anOther) const
    {
      return myDescriptorOffsetInTable == anOther.myDescriptorOffsetInTable &&
        myRootParamIndex == anOther.myRootParamIndex;
    }

    uint64 myNameHash = 0ull;  // The name of the resource in the shader source
    String myName;
    ShaderResourceTypeDX12 myType = ShaderResourceTypeDX12::None;
    bool myIsDescriptorTableEntry = false;
    uint myDescriptorOffsetInTable = UINT_MAX;
    uint myRootParamIndex = UINT_MAX;
    uint myNumDescriptors = 0u;
  };
//---------------------------------------------------------------------------// 
}
