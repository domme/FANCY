#pragma once

#include "FancyCorePrerequisites.h"
#include "MathUtil.h"
#include "GpuProgramDesc.h"

namespace Fancy {
//---------------------------------------------------------------------------//  
  struct GpuProgramPipelineDesc
  {
    GpuProgramPipelineDesc();
    bool operator==(const GpuProgramPipelineDesc& anOther) const;
    uint64 GetHash() const;
    GpuProgramDesc myGpuPrograms[(uint)ShaderStage::NUM];
  };
//---------------------------------------------------------------------------//
  inline GpuProgramPipelineDesc::GpuProgramPipelineDesc()
  {
    for (uint i = 0u; i < (uint)ShaderStage::NUM; ++i)
      myGpuPrograms[i].myShaderStage = i;
  }
//---------------------------------------------------------------------------//
  inline bool GpuProgramPipelineDesc::operator==(const GpuProgramPipelineDesc& anOther) const
  {
    bool equal = true;
    for (uint i = 0u; equal && i < (uint)ShaderStage::NUM; ++i)
      equal &= myGpuPrograms[i] == anOther.myGpuPrograms[i];
    return equal;
  }
//---------------------------------------------------------------------------//
  inline uint64 GpuProgramPipelineDesc::GetHash() const
  {
    uint64 hash = 0u;

    for (uint i = 0u; i < (uint)ShaderStage::NUM; ++i)
      MathUtil::hash_combine(hash, myGpuPrograms[i].GetHash());

    return hash;
  }
//---------------------------------------------------------------------------//
}