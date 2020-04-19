#pragma once

#include "FancyCoreDefines.h"
#include "MathUtil.h"
#include "RenderEnums.h"
#include "DynamicArray.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct ShaderDesc
  {
    uint64 GetHash() const;
    
    String myPath;
    String myMainFunction = "main";
    uint myShaderStage = (uint) ShaderStage::NONE;
    DynamicArray<String> myDefines;
  };
//---------------------------------------------------------------------------//
  inline uint64 ShaderDesc::GetHash() const
  {
    uint64 hash;
    MathUtil::hash_combine(hash, MathUtil::Hash(myPath));
    MathUtil::hash_combine(hash, myShaderStage);
    MathUtil::hash_combine(hash, MathUtil::Hash(myMainFunction));
    for (const String& define : myDefines)
      MathUtil::hash_combine(hash, define);
    return hash;
  }
//---------------------------------------------------------------------------//
}

