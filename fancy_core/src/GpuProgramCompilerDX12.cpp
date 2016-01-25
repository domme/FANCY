#include "StdAfx.h"
#include "GpuProgramCompilerUtils.h"
#include "StringUtil.h"
#include "GpuProgramFeatures.h"
#include "GpuProgram.h"

#if defined (RENDERER_DX12)

#include "GpuProgramCompilerDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  bool GpuProgramCompilerDX12::Compile(const GpuProgramDesc& aDesc, GpuProgramCompilerOutputDX12* aProgram)
  {
    return true;
  }
//---------------------------------------------------------------------------//
  GpuProgram* GpuProgramCompilerDX12::createOrRetrieve(const GpuProgramDesc& aDesc)
  {
    String shaderFilePath = aDesc.myShaderPath;
    ShaderStage shaderStage = static_cast<ShaderStage>(aDesc.myShaderStage);

    GpuProgram* pGpuProgram = GpuProgram::FindFromDesc(aDesc);
    if (pGpuProgram != nullptr)
    {
      return pGpuProgram;
    }

    pGpuProgram = FANCY_NEW(GpuProgram, MemoryCategory::MATERIALS);
    if (!pGpuProgram->SetFromDescription(aDesc))
    {
      FANCY_DELETE(pGpuProgram, MemoryCategory::MATERIALS);
      return nullptr;
    }
    GpuProgram::Register(pGpuProgram);
    return pGpuProgram;
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
