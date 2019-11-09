#pragma once

#include "FancyCoreDefines.h"
#include "MathUtil.h"
#include "ShaderDesc.h"
#include "RenderEnums.h"

namespace Fancy {
//---------------------------------------------------------------------------//  
  struct ShaderPipelineDesc
  {
    ShaderPipelineDesc();
    bool operator==(const ShaderPipelineDesc& anOther) const;
    uint64 GetHash() const;
    ShaderDesc myShader[(uint)ShaderStage::NUM];
  };
//---------------------------------------------------------------------------//
  inline ShaderPipelineDesc::ShaderPipelineDesc()
  {
    for (uint i = 0u; i < (uint)ShaderStage::NUM; ++i)
      myShader[i].myShaderStage = i;
  }
//---------------------------------------------------------------------------//
  inline bool ShaderPipelineDesc::operator==(const ShaderPipelineDesc& anOther) const
  {
    return GetHash() == anOther.GetHash();
  }
//---------------------------------------------------------------------------//
  inline uint64 ShaderPipelineDesc::GetHash() const
  {
    uint64 hash = 0u;

    for (uint i = 0u; i < (uint)ShaderStage::NUM; ++i)
      MathUtil::hash_combine(hash, myShader[i].GetHash());

    return hash;
  }
//---------------------------------------------------------------------------//
}