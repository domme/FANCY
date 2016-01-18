#pragma once
#include "FancyCorePrerequisites.h"
#include "GpuProgramFeatures.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct GpuProgramDesc
  {
    GpuProgramDesc() : myShaderPath(""), myShaderStage(0u) {}
    bool operator==(const GpuProgramDesc& anOther) const;
    uint64 GetHash() const;

    String myShaderPath;
    uint myShaderStage;
    GpuProgramPermutation myPermutation;
  };
//---------------------------------------------------------------------------//
  inline bool GpuProgramDesc::operator==(const GpuProgramDesc& anOther) const
  {
    return myShaderPath == anOther.myShaderPath
      && myShaderStage == anOther.myShaderStage
      && myPermutation == anOther.myPermutation;
  }
//---------------------------------------------------------------------------//
  inline uint64 GpuProgramDesc::GetHash() const
  {
    uint64 hash;
    MathUtil::hash_combine(hash, MathUtil::hashFromString(myShaderPath));
    MathUtil::hash_combine(hash, myShaderStage);
    MathUtil::hash_combine(hash, myPermutation.getHash());
    return hash;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//  
  struct GpuProgramPipelineDesc
  {
    bool operator==(const GpuProgramPipelineDesc& anOther) const;
    uint64 GetHash() const;
    
    GpuProgramDesc myGpuPrograms[(uint32)ShaderStage::NUM];
  };
//---------------------------------------------------------------------------//
  inline bool GpuProgramPipelineDesc::operator==(const GpuProgramPipelineDesc& anOther) const
  {
    bool equal = true;
    for (uint32 i = 0u; equal && i < (uint32)ShaderStage::NUM; ++i)
      equal &= myGpuPrograms[i] == anOther.myGpuPrograms[i];
    return equal;
  }
//---------------------------------------------------------------------------//
  inline uint64 GpuProgramPipelineDesc::GetHash() const
  {
    uint64 hash;

    for (uint32 i = 0u; i < (uint32)ShaderStage::NUM; ++i)
      MathUtil::hash_combine(hash,myGpuPrograms[i].GetHash());

    return hash;
  }
//---------------------------------------------------------------------------//
} }

