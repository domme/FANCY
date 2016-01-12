#include "StdAfx.h"
#include "GpuProgramCompilerUtils.h"
#include "StringUtil.h"
#include "GpuProgramFeatures.h"
#include "GpuProgram.h"

#if defined (RENDERER_DX12)

#include "GpuProgramCompilerDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
// TODO: Find a nicer place for platform-dependent infos
  String locShaderFileExtension = ".hlsl";
  String locShaderDirectory = "shader/DX12/";
//---------------------------------------------------------------------------//
  GpuProgram* GpuProgramCompilerDX12::createOrRetrieve(const String& aShaderFileName, const GpuProgramPermutation& aPermutation, ShaderStage aShaderStage)
  {
    String shaderFilePath = locShaderDirectory + aShaderFileName + locShaderFileExtension;

    String uniqueProgramName = aShaderFileName + "_" + GpuProgramCompilerUtils::ShaderStageToDefineString(aShaderStage) + "_" + StringUtil::toString(aPermutation.getHash());
    GpuProgram* pGpuProgram = GpuProgram::getByName(uniqueProgramName);
    if (pGpuProgram != nullptr)
    {
      return pGpuProgram;
    }

    log_Info("Compiling shader " + aShaderFileName + " ...");

    // TODO: Check the cache--folder for a precompiled shader binary

  


    return nullptr;
  }

  GpuProgramCompilerDX12::GpuProgramCompilerDX12()
  {
  }

  GpuProgramCompilerDX12::~GpuProgramCompilerDX12()
  {
  }

//---------------------------------------------------------------------------//
} } }

#endif
