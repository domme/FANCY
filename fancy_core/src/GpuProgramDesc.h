#pragma once
#include "FancyCorePrerequisites.h"
#include "GpuProgramFeatures.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct GpuProgramDesc
  {
    GpuProgramDesc() : myShaderPath(""), myShaderStage(0u) {}
    bool operator==(const GpuProgramDesc& anOther) const;

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
  struct GpuProgramPipelineDesc
  {
    bool operator==(const GpuProgramPipelineDesc& anOther) const;
    
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
} }

