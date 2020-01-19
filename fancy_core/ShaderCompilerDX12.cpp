#include "fancy_core_precompile.h"
#include "ShaderCompilerDX12.h"

#include "Shader.h"
#include "PathService.h"

#include "ShaderPipeline.h"

#include "ShaderDX12.h"
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
      inputElem.mySemantics = ShaderDX12::GetVertexSemanticFromShaderString(paramDesc.SemanticName);
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
  bool locIsRwResource(D3D_SHADER_INPUT_TYPE aType)
  {
    switch (aType)
    {
      case D3D_SIT_UAV_RWTYPED: 
      case D3D_SIT_UAV_RWSTRUCTURED: 
      case D3D_SIT_UAV_RWBYTEADDRESS: 
      case D3D11_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
        return true;
      default: return false;
    }
  }
//---------------------------------------------------------------------------//
  bool locAreResourceTypesEqual(D3D_SHADER_INPUT_TYPE anInputType, D3D12_DESCRIPTOR_RANGE_TYPE aRangeType)
  {
    switch (anInputType)
    {
      case D3D_SIT_CBUFFER: 
        return aRangeType == D3D12_DESCRIPTOR_RANGE_TYPE_CBV;

      case D3D_SIT_TBUFFER:
      case D3D_SIT_TEXTURE:
      case D3D_SIT_STRUCTURED:
      case D3D_SIT_BYTEADDRESS:
        return aRangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

      case D3D_SIT_SAMPLER:
        return aRangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;

      case D3D_SIT_UAV_RWTYPED:
      case D3D_SIT_UAV_RWSTRUCTURED:
      case D3D_SIT_UAV_RWBYTEADDRESS:
      case D3D_SIT_UAV_APPEND_STRUCTURED:
      case D3D_SIT_UAV_CONSUME_STRUCTURED:
      case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
        return aRangeType == D3D12_DESCRIPTOR_RANGE_TYPE_UAV;

      default: return false;
    }
  }
//---------------------------------------------------------------------------//
  bool locAreResourceTypesEqual(D3D_SHADER_INPUT_TYPE anInputType, D3D12_ROOT_PARAMETER_TYPE aParamType)
  {
    switch (anInputType)
    {
    case D3D_SIT_CBUFFER:
      return aParamType == D3D12_ROOT_PARAMETER_TYPE_CBV;

    case D3D_SIT_TBUFFER:
    case D3D_SIT_TEXTURE:
    case D3D_SIT_STRUCTURED:
    case D3D_SIT_BYTEADDRESS:
      return aParamType == D3D12_ROOT_PARAMETER_TYPE_SRV;

    case D3D_SIT_UAV_RWTYPED:
    case D3D_SIT_UAV_RWSTRUCTURED:
    case D3D_SIT_UAV_RWBYTEADDRESS:
    case D3D_SIT_UAV_APPEND_STRUCTURED:
    case D3D_SIT_UAV_CONSUME_STRUCTURED:
    case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
      return aParamType == D3D12_ROOT_PARAMETER_TYPE_UAV;

    default: return false;
    }
  }
//---------------------------------------------------------------------------//
  bool locAddShaderResourceInfo(const D3D12_SHADER_INPUT_BIND_DESC& aResourceDesc, const D3D12_ROOT_SIGNATURE_DESC1& aRsDesc, ShaderResourceInfoContainerDX12& someResourceInfos)
  {
    if (aResourceDesc.Type == D3D_SIT_SAMPLER) // This could be a static sampler that doesn't need an entry in the resourceInfos since its just defined in the root signature
    {
      for (uint i = 0u; i < aRsDesc.NumStaticSamplers; ++i)
      {
        if (aResourceDesc.BindPoint == aRsDesc.pStaticSamplers[i].ShaderRegister)
          return true;  // Ignore this resource - not an actual resource that needs binding from the app
      }
    }

    ShaderResourceInfoDX12 resourceInfo;
    resourceInfo.myNameHash = MathUtil::Hash(aResourceDesc.Name);
    resourceInfo.myName = aResourceDesc.Name;

    for (uint iRootParam = 0u; iRootParam < aRsDesc.NumParameters; ++iRootParam)
    {
      const D3D12_ROOT_PARAMETER1& rParam = aRsDesc.pParameters[iRootParam];
      if (rParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
      {
        const D3D12_ROOT_DESCRIPTOR_TABLE1& rDescTable = rParam.DescriptorTable;
        uint descriptorOffsetInTable = 0u;
        for (uint iDescRange = 0u; iDescRange < rDescTable.NumDescriptorRanges; ++iDescRange)
        {
          const D3D12_DESCRIPTOR_RANGE1& descRange = rDescTable.pDescriptorRanges[iDescRange];

          if (descRange.BaseShaderRegister == aResourceDesc.BindPoint && locAreResourceTypesEqual(aResourceDesc.Type, descRange.RangeType))
          {
            resourceInfo.myIsDescriptorTableEntry = true;
            resourceInfo.myRootParamIndex = iRootParam;
            resourceInfo.myDescriptorOffsetInTable = descRange.OffsetInDescriptorsFromTableStart == D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND ? descriptorOffsetInTable : descRange.OffsetInDescriptorsFromTableStart;

            switch (descRange.RangeType)
            {
              case D3D12_DESCRIPTOR_RANGE_TYPE_SRV: 
                someResourceInfos.mySRVs.push_back(resourceInfo); break;
              case D3D12_DESCRIPTOR_RANGE_TYPE_UAV: 
                someResourceInfos.myUAVs.push_back(resourceInfo); break;
              case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
                someResourceInfos.myCBVs.push_back(resourceInfo); break;
              case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER: 
                someResourceInfos.mySamplers.push_back(resourceInfo); break;
              default: return false;
            }

            return true;
          }

          descriptorOffsetInTable += descRange.NumDescriptors;
        }
      }
      else if (rParam.ParameterType != D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
      {
        if (rParam.Descriptor.ShaderRegister == aResourceDesc.BindPoint && locAreResourceTypesEqual(aResourceDesc.Type, rParam.ParameterType))
        {
          resourceInfo.myIsDescriptorTableEntry = false;
          resourceInfo.myRootParamIndex = iRootParam;

          switch (rParam.ParameterType)
          {
            case D3D12_ROOT_PARAMETER_TYPE_CBV:
              someResourceInfos.myCBVs.push_back(resourceInfo); break;
            case D3D12_ROOT_PARAMETER_TYPE_SRV:
              someResourceInfos.mySRVs.push_back(resourceInfo); break;
            case D3D12_ROOT_PARAMETER_TYPE_UAV:
              someResourceInfos.myUAVs.push_back(resourceInfo); break;
            default: return false;
          }

          return true;
        }
      }
      else
      {
        LOG_ERROR("Unsupported root parameter type for resource %s. FANCY supports only CBVs as root descriptors and doesn't support any root constants", aResourceDesc.Name);
      }
    }

    return false;
  }
//---------------------------------------------------------------------------//
  bool locReflectResources(ID3D12ShaderReflection* aReflector, const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* aRootSignatureDesc, 
    ShaderResourceInfoContainerDX12& someResourceInfosOut, bool& aHasUnorderedWritesOut)
  {
    ASSERT(aRootSignatureDesc->Version == D3D_ROOT_SIGNATURE_VERSION_1_1);
    const D3D12_ROOT_SIGNATURE_DESC1& rsDesc = aRootSignatureDesc->Desc_1_1;

    D3D12_SHADER_DESC shaderDesc;
    aReflector->GetDesc(&shaderDesc);

    bool hasUnorderedWrites = false;
    for (uint i = 0u; i < shaderDesc.BoundResources && !hasUnorderedWrites; ++i)
    {
      D3D12_SHADER_INPUT_BIND_DESC resourceDesc;
      CheckD3Dcall(aReflector->GetResourceBindingDesc(i, &resourceDesc));

      hasUnorderedWrites |= locIsRwResource(resourceDesc.Type);

      if (!locAddShaderResourceInfo(resourceDesc, rsDesc, someResourceInfosOut))
        return false;
    }
    aHasUnorderedWritesOut = hasUnorderedWrites;

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

    ShaderCompiledDataDX12 compiledNativeData;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    sucess = d3dDevice->CreateRootSignature(0u, rsBlob->GetBufferPointer(), rsBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
    compiledNativeData.myRootSignature = rootSignature;

    if (S_OK != sucess)
    {
      LOG_ERROR("Failed creating the root signature from shader");
      return false;
    }

    Microsoft::WRL::ComPtr<ID3D12VersionedRootSignatureDeserializer> rsDeserializer;
    sucess = D3D12CreateVersionedRootSignatureDeserializer(rsBlob->GetBufferPointer(), rsBlob->GetBufferSize(), IID_PPV_ARGS(&rsDeserializer));

    if (S_OK != sucess)
    {
      LOG_ERROR("Failed deserializing the shader root signature");
      return false;
    }

    const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* rsDesc = rsDeserializer->GetUnconvertedRootSignatureDesc();

    // Reflect the shader resources
    //---------------------------------------------------------------------------//
    ID3D12ShaderReflection* reflector;
    sucess = D3DReflect(compiledShaderBytecode->GetBufferPointer(), compiledShaderBytecode->GetBufferSize(), IID_ID3D12ShaderReflection, (void**)&reflector);

    if (S_OK != sucess)
    {
      LOG_ERROR("Failed reflecting shader");
      return false;
    }

    if (!locReflectResources(reflector, rsDesc, compiledNativeData.myResourceInfos, anOutput->myProperties.myHasUnorderedWrites))
    {
      LOG_ERROR("Failed reflecting shader resources");
      return false;
    }

    if (aDesc.myShaderStage == static_cast<uint>(ShaderStage::VERTEX))
    {
      D3D12_SHADER_DESC shaderDesc;
      reflector->GetDesc(&shaderDesc);

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
        
    compiledNativeData.myBytecodeBlob = compiledShaderBytecode.Detach();  // TODO: Find a safer way to manage this to avoid leaks...
    anOutput->myNativeData = compiledNativeData;

    return true;
  }
//---------------------------------------------------------------------------//
}
