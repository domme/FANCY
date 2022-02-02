#pragma once

#include "Common/FancyCoreDefines.h"
#include "Common/MathUtil.h"
#include "RenderEnums.h"

#include "EASTL/fixed_vector.h"
#include "EASTL/fixed_string.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct ShaderDesc
  {
    uint64 GetHash() const;
    
    eastl::string myPath;
    eastl::string myMainFunction = "main";
    uint myShaderStage = (uint) SHADERSTAGE_NONE;
    eastl::fixed_vector<eastl::string, 8> myDefines;
  };
//---------------------------------------------------------------------------//
  inline uint64 ShaderDesc::GetHash() const
  {
    uint64 hash;
    MathUtil::hash_combine(hash, MathUtil::Hash(myPath));
    MathUtil::hash_combine(hash, myShaderStage);
    MathUtil::hash_combine(hash, MathUtil::Hash(myMainFunction));
    for (const eastl::string& define : myDefines)
      MathUtil::hash_combine(hash, define);
    return hash;
  }
//---------------------------------------------------------------------------//
}

