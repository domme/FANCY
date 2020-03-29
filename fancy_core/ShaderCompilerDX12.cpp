#include "fancy_core_precompile.h"
#include "ShaderCompilerDX12.h"

#include "Shader.h"
#include "PathService.h"

#include "ShaderPipeline.h"

#include "ShaderDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"

#include <dxc/dxcapi.h>
#include <dxc/DxilContainer/DxilContainer.h>

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
  ShaderResourceTypeDX12 locGetShaderResourceInfoType(D3D12_ROOT_PARAMETER_TYPE aRootParamType)
  {
    switch (aRootParamType) 
    { 
      case D3D12_ROOT_PARAMETER_TYPE_CBV: return ShaderResourceTypeDX12::CBV;
      case D3D12_ROOT_PARAMETER_TYPE_SRV: return ShaderResourceTypeDX12::SRV;
      case D3D12_ROOT_PARAMETER_TYPE_UAV: return ShaderResourceTypeDX12::UAV;
      default: ASSERT(false); return ShaderResourceTypeDX12::CBV;
    }
  }
//---------------------------------------------------------------------------//
  ShaderResourceTypeDX12 locGetShaderResourceInfoType(D3D12_DESCRIPTOR_RANGE_TYPE aRangeType)
  {
    switch (aRangeType)
    {
    case D3D12_DESCRIPTOR_RANGE_TYPE_CBV: return ShaderResourceTypeDX12::CBV;
    case D3D12_DESCRIPTOR_RANGE_TYPE_SRV: return ShaderResourceTypeDX12::SRV;
    case D3D12_DESCRIPTOR_RANGE_TYPE_UAV: return ShaderResourceTypeDX12::UAV;
    case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER: return ShaderResourceTypeDX12::Sampler;
    default: ASSERT(false); return ShaderResourceTypeDX12::CBV;
    }
  }
//---------------------------------------------------------------------------//
  bool locAddShaderResourceInfo(const D3D12_SHADER_INPUT_BIND_DESC& aResourceDesc, const D3D12_ROOT_SIGNATURE_DESC1& aRsDesc, DynamicArray<ShaderResourceInfoDX12>& someResourceInfos)
  {
    if (aResourceDesc.Type == D3D_SIT_SAMPLER) // This could be a static sampler that doesn't need an entry in the resourceInfos since its just defined in the root signature
    {
      for (uint i = 0u; i < aRsDesc.NumStaticSamplers; ++i)
      {
        if (aResourceDesc.BindPoint == aRsDesc.pStaticSamplers[i].ShaderRegister)
          return true;  // Ignore this resource - not an actual resource that needs binding from the app
      }
    }

    const char* name = aResourceDesc.Name;

    ShaderResourceInfoDX12 resourceInfo;
    resourceInfo.myNameHash = MathUtil::Hash(name);
    resourceInfo.myName = name;

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
            ASSERT(descriptorOffsetInTable != UINT_MAX || descRange.OffsetInDescriptorsFromTableStart != D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND, "Ranges following an unbounded range must have an explicit offset");
            resourceInfo.myDescriptorOffsetInTable = descRange.OffsetInDescriptorsFromTableStart == D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND ? descriptorOffsetInTable : descRange.OffsetInDescriptorsFromTableStart;
            resourceInfo.myType = locGetShaderResourceInfoType(descRange.RangeType);
            resourceInfo.myNumDescriptors = descRange.NumDescriptors;

            someResourceInfos.push_back(resourceInfo);

            return true;
          }

          if (descRange.NumDescriptors == UINT_MAX)  // Unbounded range. Usually appears as the last range in the table definition
            descriptorOffsetInTable = UINT_MAX;
          else
            descriptorOffsetInTable += descRange.NumDescriptors;
        }
      }
      else if (rParam.ParameterType != D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
      {
        if (rParam.Descriptor.ShaderRegister == aResourceDesc.BindPoint && locAreResourceTypesEqual(aResourceDesc.Type, rParam.ParameterType))
        {
          resourceInfo.myIsDescriptorTableEntry = false;
          resourceInfo.myRootParamIndex = iRootParam;
          resourceInfo.myType = locGetShaderResourceInfoType(rParam.ParameterType);
          resourceInfo.myNumDescriptors = 1u;

          someResourceInfos.push_back(resourceInfo);

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
    DynamicArray<ShaderResourceInfoDX12>& someResourceInfosOut, bool& aHasUnorderedWritesOut)
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
  bool ShaderCompilerDX12::Compile_Internal(const char* anHlslSrcPathAbs, const ShaderDesc& aDesc, ShaderCompilerResult* anOutput) const
  {
    DxcShaderCompiler::Config config =
    {
      true,
      false,
      GetHLSLprofileString((ShaderStage)aDesc.myShaderStage)
    };

    Microsoft::WRL::ComPtr<IDxcBlob> compiledShaderBytecode;
    if (!myDxcCompiler.CompileToBytecode(anHlslSrcPathAbs, aDesc, config, compiledShaderBytecode))
      return false;

    IDxcContainerReflection* dxcReflection = myDxcCompiler.GetDxcReflector();
    HRESULT success = dxcReflection->Load(compiledShaderBytecode.Get());
    if (success != S_OK)
    {
      LOG_ERROR("Failed to load the compiled shader bytecode into the dxc reflector");
      return false;
    }

    // Extract and parse RootSignature
    uint rootSigPartIdx;
    success = dxcReflection->FindFirstPartKind(hlsl::DFCC_RootSignature, &rootSigPartIdx);
    ASSERT(success == S_OK);

    ID3D12Device* d3dDevice = RenderCore::GetPlatformDX12()->GetDevice();

    ShaderCompiledDataDX12 compiledNativeData;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    success = d3dDevice->CreateRootSignature(0u, compiledShaderBytecode->GetBufferPointer(), compiledShaderBytecode->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
    compiledNativeData.myRootSignature = rootSignature;
    if (S_OK != success)
    {
      LOG_ERROR("Failed creating the root signature from shader");
      return false;
    }

    Microsoft::WRL::ComPtr<ID3D12VersionedRootSignatureDeserializer> rsDeserializer;
    success = D3D12CreateVersionedRootSignatureDeserializer(compiledShaderBytecode->GetBufferPointer(), compiledShaderBytecode->GetBufferSize(), IID_PPV_ARGS(&rsDeserializer));
    if (S_OK != success)
    {
      LOG_ERROR("Failed deserializing the shader root signature");
      return false;
    }

    const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* rsDesc = rsDeserializer->GetUnconvertedRootSignatureDesc();

    // Reflect the shader resources
    //---------------------------------------------------------------------------//
    uint dxilPartIdx;
    success = dxcReflection->FindFirstPartKind(hlsl::DFCC_DXIL, &dxilPartIdx);
    ASSERT(success == S_OK);

    Microsoft::WRL::ComPtr<ID3D12ShaderReflection> reflector;
    success = dxcReflection->GetPartReflection(dxilPartIdx, IID_PPV_ARGS(&reflector));
    if (S_OK != success)
    {
      LOG_ERROR("Failed reflecting shader");
      return false;
    }

    if (!locReflectResources(reflector.Get(), rsDesc, compiledNativeData.myResourceInfos, anOutput->myProperties.myHasUnorderedWrites))
    {
      LOG_ERROR("Failed reflecting shader resources");
      return false;
    }

    if (aDesc.myShaderStage == static_cast<uint>(ShaderStage::VERTEX))
    {
      D3D12_SHADER_DESC shaderDesc;
      reflector->GetDesc(&shaderDesc);

      if (!locReflectVertexInputLayout(reflector.Get(), shaderDesc, anOutput->myProperties))
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
    
    compiledNativeData.myBytecode.resize(compiledShaderBytecode->GetBufferSize());
    memcpy(compiledNativeData.myBytecode.data(), compiledShaderBytecode->GetBufferPointer(), compiledShaderBytecode->GetBufferSize());

    anOutput->myNativeData = compiledNativeData;

    return true;
  }
//---------------------------------------------------------------------------//
}
