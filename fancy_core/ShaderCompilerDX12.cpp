#include "fancy_core_precompile.h"
#include "ShaderCompilerDX12.h"

#include "Shader.h"
#include "PathService.h"
#include "ShaderResourceInterface.h"

#include "ShaderPipeline.h"

#include "GpuProgramDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  DataFormat locResolveFormat(const D3D12_SIGNATURE_PARAMETER_DESC& aParamDesc)
  {
    switch(aParamDesc.ComponentType)
    {
      case D3D_REGISTER_COMPONENT_UINT32: 
      {
        if (aParamDesc.Mask <= 1) return DataFormat::R_32UI;
        else if (aParamDesc.Mask <= 3) return DataFormat::RG_32UI;
        else if (aParamDesc.Mask <= 7) return DataFormat::RGB_32UI;
        else if (aParamDesc.Mask <= 15) return DataFormat::RGBA_32UI;
      }
      break;

      case D3D_REGISTER_COMPONENT_SINT32: 
      {
        if (aParamDesc.Mask <= 1) return DataFormat::R_32I;
        else if (aParamDesc.Mask <= 3) return DataFormat::RG_32I;
        else if (aParamDesc.Mask <= 7) return DataFormat::RGB_32I;
        else if (aParamDesc.Mask <= 15) return DataFormat::RGBA_32I;
      }
      break;

      case D3D_REGISTER_COMPONENT_FLOAT32: 
      {
        if (aParamDesc.Mask <= 1) return DataFormat::R_32F;
        else if (aParamDesc.Mask <= 3) return DataFormat::RG_32F;
        else if (aParamDesc.Mask <= 7) return DataFormat::RGB_32F;
        else if (aParamDesc.Mask <= 15) return DataFormat::RGBA_32F;
      }
      break;
    }

    ASSERT(false, "Component type not implemented");
    return DataFormat::NONE;
  }
//---------------------------------------------------------------------------//
  uint locResolveSizeBytes(const D3D12_SIGNATURE_PARAMETER_DESC& aParamDesc)
  {
    switch (aParamDesc.ComponentType)
    {
      case D3D_REGISTER_COMPONENT_SINT32:
      case D3D_REGISTER_COMPONENT_UINT32:
      case D3D_REGISTER_COMPONENT_FLOAT32:
      {
        if (aParamDesc.Mask <= 1) return 4u;
        else if (aParamDesc.Mask <= 3) return 8u;
        else if (aParamDesc.Mask <= 7) return 12u;
        else if (aParamDesc.Mask <= 15) return 16u;
      }
      break;
    }

    ASSERT(false, "Component type not implemented");
    return 0u;
  }
//---------------------------------------------------------------------------//
  uint locResolveComponentCount(const D3D12_SIGNATURE_PARAMETER_DESC& aParamDesc)
  {
    if (aParamDesc.Mask <= 1) return 1u;
    else if (aParamDesc.Mask <= 3) return 2u;
    else if (aParamDesc.Mask <= 7) return 3u;
    else if (aParamDesc.Mask <= 15) return 4u;

    ASSERT(false, "Unknown param-mask");
    return 0u;
  }
//---------------------------------------------------------------------------//
  void locResolveFormat(const char* aTypeName, uint& aSizeBytesOut, DataFormat& aDataFormatOut)
  {
    uint sizeBytes = 0u;
    DataFormat format = DataFormat::NONE;

#define CHECK(str) strcmp(aTypeName, str) == 0
    
    const uint kFloatSize = 4u;
    const uint kUintSize = 4u;
    const uint kIntSize = 4u;

    if (CHECK("float")) 
    {
      sizeBytes = kFloatSize; 
      format = DataFormat::R_32F;
    } 
    else if (CHECK("float2"))
    {
      sizeBytes = 2u * kFloatSize;
      format = DataFormat::RG_32F;
    }
    else if (CHECK("float3"))
    {
      sizeBytes = 3u * kFloatSize;
      format = DataFormat::RGB_32F;
    }
    else if (CHECK("float4"))
    {
      sizeBytes = 4u * kFloatSize;
      format = DataFormat::RGBA_32F;
    }
    else if(CHECK("float2x2"))
    {
      sizeBytes = 2 * 2 * kFloatSize;
      format = DataFormat::RG_32F;
    }
    else if(CHECK("float3x3"))
    {
      sizeBytes = 3 * 3 * kFloatSize;
      format = DataFormat::RGB_32F;
    }
    else if(CHECK("float4x4"))
    {
      sizeBytes = 4 * 4 * kFloatSize;
      format = DataFormat::RGBA_32F;
    }

    else if (CHECK("uint") || CHECK("dword"))
    {
      sizeBytes = kUintSize;
      format = DataFormat::R_32UI;
    }
    else if (CHECK("uint2"))
    {
      sizeBytes = 2u * kUintSize;
      format = DataFormat::RG_32UI;
    }
    else if (CHECK("uint3"))
    {
      sizeBytes = 3u * kUintSize;
      format = DataFormat::RGB_32UI;
    }
    else if (CHECK("uint4"))
    {
      sizeBytes = 4u * kUintSize;
      format = DataFormat::RGBA_32UI;
    }
    else if (CHECK("uint2x2"))
    {
      sizeBytes = 2 * 2 * kUintSize;
      format = DataFormat::RG_32UI;
    }
    else if (CHECK("uint3x3"))
    {
      sizeBytes = 3 * 3 * kUintSize;
      format = DataFormat::RGB_32UI;
    }
    else if (CHECK("uint4x4"))
    {
      sizeBytes = 4 * 4 * kUintSize;
      format = DataFormat::RGBA_32UI;
    }
    
    else if (CHECK("int"))
    {
      sizeBytes = kIntSize;
      format = DataFormat::R_32UI;
    }
    else if (CHECK("int2"))
    {
      sizeBytes = 2u * kIntSize;
      format = DataFormat::RG_32UI;
    }
    else if (CHECK("int3"))
    {
      sizeBytes = 3u * kIntSize;
      format = DataFormat::RGB_32UI;
    }
    else if (CHECK("int4"))
    {
      sizeBytes = 4u * kIntSize;
      format = DataFormat::RGBA_32UI;
    }
    else if (CHECK("int2x2"))
    {
      sizeBytes = 2 * 2 * kIntSize;
      format = DataFormat::RG_32UI;
    }
    else if (CHECK("int3x3"))
    {
      sizeBytes = 3 * 3 * kIntSize;
      format = DataFormat::RGB_32UI;
    }
    else if (CHECK("int4x4"))
    {
      sizeBytes = 4 * 4 * kIntSize;
      format = DataFormat::RGBA_32UI;
    }
    else
    {
      ASSERT(false, "Unexpected HLSL format");
    }

    aSizeBytesOut = sizeBytes;
    aDataFormatOut = format;

#undef CHECK
  }
//---------------------------------------------------------------------------//
  bool locReflectVertexInputLayout(ID3D12ShaderReflection* aReflector, 
    const D3D12_SHADER_DESC& aShaderDesc, ShaderProperties& someProps)
  {
    if (aShaderDesc.InputParameters == 0u)
      return false;

    someProps.myVertexInputLayout.myVertexInputElements.clear();
    someProps.myVertexInputLayout.myVertexInputElements.reserve(aShaderDesc.InputParameters);

    D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
    for (uint i = 0u; i < aShaderDesc.InputParameters; ++i)
    {
      aReflector->GetInputParameterDesc(i, &paramDesc);
      
      ShaderVertexInputElement inputElem;
      inputElem.mySemantics = GpuProgramDX12::GetVertexSemanticFromShaderString(paramDesc.SemanticName);
      inputElem.mySemanticIndex = paramDesc.SemanticIndex;
      inputElem.myFormat = locResolveFormat(paramDesc);
      inputElem.mySizeBytes = locResolveSizeBytes(paramDesc);
      inputElem.myName = paramDesc.SemanticName;
      inputElem.myFormatComponentCount = locResolveComponentCount(paramDesc);
      someProps.myVertexInputLayout.myVertexInputElements.push_back(inputElem);
    }

    return true;
  }
//---------------------------------------------------------------------------//
  String ShaderCompilerDX12::GetShaderPath(const char* aPath) const
  {
    const StaticFilePath path("%s/DX12/%s.hlsl", ShaderCompiler::GetShaderRootFolderRelative(), aPath);
    return String(path);
  }
//---------------------------------------------------------------------------//
  bool ShaderCompilerDX12::Compile_Internal(const ShaderDesc& aDesc, const char* aStageDefine, ShaderCompilerResult* anOutput) const
  {
    DynamicArray<D3D_SHADER_MACRO> defines;
    defines.resize(aDesc.myDefines.size() + 2u);
    defines[0].Name = aStageDefine;
    defines[0].Definition = "1";
    for (uint i = 0u, e = (uint) aDesc.myDefines.size(); i < e; ++i)
    {
      defines[i + 1].Name = aDesc.myDefines[i].c_str();
      defines[i + 1].Definition = "1";
    }
    defines[defines.size() - 1].Name = nullptr;
    defines[defines.size() - 1].Definition = nullptr;
    
    Microsoft::WRL::ComPtr<ID3DBlob> compiledShaderBytecode;
    Microsoft::WRL::ComPtr<ID3DBlob> errorData;

    const String actualShaderPath = GetShaderPath(aDesc.myShaderFileName.c_str());
    std::wstring shaderPathAbs = StringUtil::ToWideString(Resources::FindPath(actualShaderPath));

    HRESULT sucess = D3DCompileFromFile(
      shaderPathAbs.c_str(),
      &defines[0],
      D3D_COMPILE_STANDARD_FILE_INCLUDE,
      aDesc.myMainFunction.c_str(),
      GetHLSLprofileString(static_cast<ShaderStage>(aDesc.myShaderStage)),
      D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_OPTIMIZATION_LEVEL0 | D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_WARNINGS_ARE_ERRORS,
      0u,
      &compiledShaderBytecode,
      &errorData);

    if (S_OK != sucess)
    {
      if (errorData != nullptr)
      {
        const char* errorMsg = (const char*)errorData->GetBufferPointer();
        LOG_WARNING(errorMsg);
        errorData.ReleaseAndGetAddressOf();
      }

      return false;
    }

    // Extract and parse RootSignature
    ID3DBlob* rsBlob = nullptr;
    sucess = D3DGetBlobPart(compiledShaderBytecode->GetBufferPointer(), compiledShaderBytecode->GetBufferSize(), D3D_BLOB_ROOT_SIGNATURE, 0u, &rsBlob);

    if (S_OK != sucess)
    {
      LOG_ERROR("Failed extracting the root signature from shader");
      return false;
    }

    ID3D12Device* d3dDevice = RenderCore::GetPlatformDX12()->GetDevice();

    ID3D12RootSignature* rootSignature;
    sucess = d3dDevice->CreateRootSignature(0u, rsBlob->GetBufferPointer(), rsBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

    if (S_OK != sucess)
    {
      LOG_ERROR("Failed creating the root signature from shader");
      return false;
    }

    Microsoft::WRL::ComPtr<ID3D12RootSignatureDeserializer> rsDeserializer;
    sucess = D3D12CreateRootSignatureDeserializer(rsBlob->GetBufferPointer(), rsBlob->GetBufferSize(), IID_PPV_ARGS(&rsDeserializer));

    if (S_OK != sucess)
    {
      LOG_ERROR("Failed creating a rootSignature deserializer");
      return false;
    }

    const D3D12_ROOT_SIGNATURE_DESC* rsDesc = rsDeserializer->GetRootSignatureDesc();
    ShaderResourceInterface* rsObject = RenderCore::GetPlatformDX12()->GetShaderResourceInterface(*rsDesc, rootSignature);
    ASSERT(nullptr != rsObject);

    anOutput->myDesc = aDesc;
    anOutput->myProperties.myShaderStage = static_cast<ShaderStage>(aDesc.myShaderStage);
    anOutput->myRootSignature = rsObject;

    // Reflect the shader resources
    //---------------------------------------------------------------------------//
    ID3D12ShaderReflection* reflector;
    sucess = D3DReflect(compiledShaderBytecode->GetBufferPointer(), compiledShaderBytecode->GetBufferSize(), IID_ID3D12ShaderReflection, (void**)&reflector);

    if (S_OK != sucess)
    {
      LOG_ERROR("Failed reflecting shader");
      return false;
    }

    D3D12_SHADER_DESC shaderDesc;
    reflector->GetDesc(&shaderDesc);

    bool hasUnorderedWrites = false;
    for (uint i = 0u; i < shaderDesc.BoundResources && !hasUnorderedWrites; ++i)
    {
      D3D12_SHADER_INPUT_BIND_DESC desc;
      CheckD3Dcall(reflector->GetResourceBindingDesc(i, &desc));

      hasUnorderedWrites |= (desc.Type == D3D_SIT_UAV_RWTYPED || desc.Type == D3D_SIT_UAV_RWSTRUCTURED ||
        desc.Type == D3D_SIT_UAV_RWBYTEADDRESS || desc.Type == D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER);
    }
    anOutput->myProperties.myHasUnorderedWrites = hasUnorderedWrites;
    
    if (aDesc.myShaderStage == static_cast<uint>(ShaderStage::VERTEX))
    {
      if (!locReflectVertexInputLayout(reflector, shaderDesc, anOutput->myProperties))
      {
        LOG_ERROR("Failed reflecting vertex input layout");
        return false;
      }
    }
    else if (aDesc.myShaderStage == static_cast<uint>(ShaderStage::COMPUTE))
    {
      uint x, y, z;
      reflector->GetThreadGroupSize(&x, &y, &z);
      anOutput->myProperties.myNumGroupThreads = glm::int3(static_cast<int>(x), static_cast<int>(y), static_cast<int>(z));
    }

    ShaderCompiledDataDX12 nativeData;
    nativeData.myBytecodeBlob = compiledShaderBytecode.Detach();  // TODO: Find a safer way to manage this to avoid leaks...
    anOutput->myNativeData = nativeData;

    return true;
  }
//---------------------------------------------------------------------------//
}
