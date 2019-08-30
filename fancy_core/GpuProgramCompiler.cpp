#include "fancy_core_precompile.h"
#include "GpuProgramCompiler.h"
#include "GpuProgramCompilerUtils.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  bool GpuProgramCompiler::Compile(const GpuProgramDesc& aDesc, GpuProgramCompilerOutput* aCompilerOutput)
  {
    LOG_INFO("Compiling shader %s...", aDesc.myShaderFileName.c_str());



    const String& shaderStageDefineStr = GpuProgramCompilerUtils::ShaderStageToDefineString(static_cast<ShaderStage>(aDesc.myShaderStage));
    
    featureDefineStrings[i] = GpuProgramPermutation::featureToDefineString(permuationFeatures[i]);
    const DynamicArray<GpuProgramFeature>& permuationFeatures = aDesc.myPermutation.getFeatureList();

    
    defines.resize(permuationFeatures.size() + 1u + 1u); // All permutation-defines + the stage define + termination macro

    defines[0].Name = shaderStageDefineStr.c_str();
    defines[0].Definition = "1";
  }
 //---------------------------------------------------------------------------//
}
