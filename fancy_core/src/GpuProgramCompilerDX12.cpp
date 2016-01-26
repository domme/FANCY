#include "StdAfx.h"
#include "GpuProgramCompilerUtils.h"
#include "StringUtil.h"
#include "GpuProgramFeatures.h"
#include "GpuProgram.h"
#include "PathService.h"

#if defined (RENDERER_DX12)

#include "GpuProgramCompilerDX12.h"
#include "RendererDX12.h"
#include "Renderer.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  String locShaderStageToProfileString(ShaderStage aStage)
  {
    switch(aStage)
    {
      case ShaderStage::VERTEX: return "vs_5_1";
      case ShaderStage::FRAGMENT: return "ps_5_1";
      case ShaderStage::GEOMETRY: return "gs_5_1";
      case ShaderStage::COMPUTE: return "cs_5_1";
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
    std::wstring shaderPathAbs = StringUtil::ToWideString(IO::PathService::convertToAbsPath(aDesc.myShaderPath));

    const GpuProgramFeatureList& permuationFeatures = aDesc.myPermutation.getFeatureList();

    std::vector<D3D_SHADER_MACRO> defines;
    defines.resize(permuationFeatures.size() + 1u + 1u); // All permutation-defines + the stage define + termination macro

    defines[0].Name = shaderStageDefineStr.c_str();
    defines[0].Definition = "1";

    std::vector<String> featureDefineStrings; // We need some temporary strings so that the c_str() are backed by memory until the end of the method
    featureDefineStrings.resize(permuationFeatures.size());

    for (uint i = 0u; i < permuationFeatures.size(); ++i)
    {
      featureDefineStrings[i] = GpuProgramPermutation::featureToDefineString(permuationFeatures[i]);
      defines[i + 1u].Name = featureDefineStrings[i].c_str(); 
      defines[i + 1u].Definition = "1";
    }

    defines[defines.size() - 1].Name = nullptr;
    defines[defines.size() - 1].Definition = nullptr;

    ID3DBlob* compiledShaderBytecode;
    ID3DBlob* errorData;

    HRESULT sucess = D3DCompileFromFile(
      shaderPathAbs.c_str(),
      &defines[0],
      D3D_COMPILE_STANDARD_FILE_INCLUDE,
      "main",
      shaderProfileStr.c_str(),
      D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
      0u,
      &compiledShaderBytecode,
      &errorData);

    if (S_OK != sucess)
    {
      if (errorData != nullptr)
      {
        const char* errorMsg = (const char*)errorData->GetBufferPointer();
        log_Error(errorMsg);
        errorData->Release();
      }

      return false;
    }

    ID3D12ShaderReflection* reflector;

    sucess = D3DReflect(compiledShaderBytecode->GetBufferPointer(), compiledShaderBytecode->GetBufferSize(), IID_ID3D12ShaderReflection, (void**)&reflector);

    if (S_OK != sucess)
    {
      log_Error("Failed reflecting shader");
      return false;
    }

    ID3DBlob* rsBlob = nullptr;
    sucess = D3DGetBlobPart(compiledShaderBytecode->GetBufferPointer(), compiledShaderBytecode->GetBufferSize(), D3D_BLOB_ROOT_SIGNATURE, 0u, &rsBlob);

    if (S_OK != sucess)
    {
      log_Error("Failed extracting the root signature from shader");
      return false;
    }
    
    ComPtr<ID3D12Device>& d3dDevice = Renderer::getInstance().GetDevice();

    ComPtr<ID3D12RootSignature> rootSignature;
    sucess = d3dDevice->CreateRootSignature(0u, rsBlob->GetBufferPointer(), rsBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

    if (S_OK != sucess)
    {
      log_Error("Failed creating the root signature from shader");
      return false;
    }

    ComPtr<ID3D12RootSignatureDeserializer> rsDeserializer;
    sucess = D3D12CreateRootSignatureDeserializer(rsBlob->GetBufferPointer(), rsBlob->GetBufferSize(), IID_PPV_ARGS(&rsDeserializer));

    if (S_OK != sucess)
    {
      log_Error("Failed creating a rootSignature deserializer");
      return false;
    }

    const D3D12_ROOT_SIGNATURE_DESC* rsDesc = rsDeserializer->GetRootSignatureDesc();

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
