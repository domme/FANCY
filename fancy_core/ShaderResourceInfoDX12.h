#pragma once

#include "DX12Prerequisites.h"

namespace Fancy
{
//---------------------------------------------------------------------------// 
  struct ShaderResourceInfoDX12
  {
    bool operator==(const ShaderResourceInfoDX12& anOther) const
    {
      return myDescriptorOffsetInTable == anOther.myDescriptorOffsetInTable &&
        myRootParamIndex == anOther.myRootParamIndex;
    }

    enum Type
    {
      None, CBV, SRV, UAV, Sampler
    };

    uint64 myNameHash = 0ull;  // The name of the resource in the shader source
    String myName;
    Type myType = None;
    bool myIsDescriptorTableEntry = false;
    uint myDescriptorOffsetInTable = UINT_MAX;
    uint myRootParamIndex = UINT_MAX;
  };
}



