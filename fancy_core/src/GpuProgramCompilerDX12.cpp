#include "StdAfx.h"
#include "GpuProgramCompilerUtils.h"
#include "StringUtil.h"
#include "GpuProgramFeatures.h"
#include "GpuProgram.h"
#include "PathService.h"

#if defined (RENDERER_DX12)

#include "GpuProgramCompilerDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  String locShaderStageToProfileString(ShaderStage aStage)
  {
    switch(aStage)
    {
      case ShaderStage::VERTEX: return "vs_5_0";
      case ShaderStage::FRAGMENT: return "ps_5_0";
      case ShaderStage::GEOMETRY: return "gs_5_0";
      case ShaderStage::COMPUTE: return "cs_5_0";
      default: 
        ASSERT_M(false, "Unsupported HLSL shader-profile");
        return "";
        break;
    }
  }
//---------------------------------------------------------------------------//
  bool GpuProgramCompilerDX12::Compile(const GpuProgramDesc& aDesc, GpuProgramCompilerOutputDX12* aProgram)
  {
    log_Info("Compiling shader " + aDesc.myShaderPath + " ...");

    const String& shaderStageDefineStr = GpuProgramCompilerUtils::ShaderStageToDefineString(static_cast<ShaderStage>(aDesc.myShaderStage));
    const String& shaderProfileStr = locShaderStageToProfileString(static_cast<ShaderStage>(aDesc.myShaderStage));
    
    D3D_SHADER_MACRO macro;
    macro.Definition = shaderStageDefineStr.c_str();
    macro.Name = "ShaderStage define";

    

    String shaderPathAbs = "C:/Users/paino/Documents/GitHub/FANCY/resources/Shader/DX12/MaterialForward.hlsl";

    std::wstring shaderPathAbsW = StringUtil::ToWideString(shaderPathAbs);

      //IO::PathService::convertToAbsPath(aDesc.myShaderPath);

    ID3DBlob* compiledShaderBytecode;
    ID3DBlob* errorData;

    HRESULT sucess = D3DCompileFromFile(
      shaderPathAbsW.c_str(),
      //nullptr,
      macros[0],
      nullptr,
      "main",
      "vs_5_0",
      D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
      0u,
      &compiledShaderBytecode,
      &errorData);

    if (S_OK != sucess)
    {
      const wchar_t* errorMsg = (const wchar_t*) errorData->GetBufferPointer();

      log_Error(errorMsg);
      return false;
    }





                                        
                                        

    return true;
  }
//---------------------------------------------------------------------------//
  GpuProgram* GpuProgramCompilerDX12::createOrRetrieve(const GpuProgramDesc& aDesc)
  {
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
