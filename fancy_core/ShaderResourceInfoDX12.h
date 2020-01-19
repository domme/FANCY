#pragma once

#include "DX12Prerequisites.h"

namespace Fancy
{
//---------------------------------------------------------------------------// 
  struct ShaderResourceInfoDX12
  {
    uint64 myNameHash;  // The name of the resource in the shader source
    String myName;
    bool myIsDescriptorTableEntry;
    uint myDescriptorOffsetInTable;
    uint myRootParamIndex;
  };
//---------------------------------------------------------------------------//
  struct ShaderResourceInfoContainerDX12
  {
    DynamicArray<ShaderResourceInfoDX12> mySRVs;
    DynamicArray<ShaderResourceInfoDX12> myCBVs;
    DynamicArray<ShaderResourceInfoDX12> myUAVs;
    DynamicArray<ShaderResourceInfoDX12> mySamplers;
  };
//---------------------------------------------------------------------------//
}



