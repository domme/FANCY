#pragma once

#include "DX12Prerequisites.h"

namespace Fancy
{
//---------------------------------------------------------------------------// 
  struct ShaderResourceInfoDX12
  {
    enum Type
    {
      CBV, SRV, UAV, Sampler
    };

    uint64 myNameHash;  // The name of the resource in the shader source
    String myName;
    bool myIsDescriptorTableEntry;
    uint myDescriptorOffsetInTable;
    uint myRootParamIndex;
  };
}



