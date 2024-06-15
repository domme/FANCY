#include "fancy_core_precompile.h"
#include "ShaderCompilerDX12.h"

#include "Rendering/Shader.h"
#include "Rendering/ShaderPipeline.h"
#include "Rendering/RenderCore.h"

#include "ShaderDX12.h"
#include "RenderCore_PlatformDX12.h"

#include "directx-dxc/dxcapi.h"

#if FANCY_ENABLE_DX12

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
  bool ShaderCompilerDX12::Compile_Internal(const char* anHlslSrcPathAbs, const ShaderDesc& aDesc, ShaderCompilerResult* anOutput) const
  {
    DxcShaderCompiler::Config config =
    {
      true,
      GetHLSLprofileString((ShaderStage)aDesc.myShaderStage).c_str()
    };

    DxcShaderCompiler::IncludeInfo includeInfo;
    Microsoft::WRL::ComPtr<IDxcBlob> compiledShaderBytecode;
    if (!myDxcCompiler.CompileToBytecode(anHlslSrcPathAbs, aDesc, config, includeInfo, compiledShaderBytecode))
      return false;
    
    IDxcContainerReflection* dxcReflection = myDxcCompiler.GetDxcReflector();
    HRESULT success = dxcReflection->Load(compiledShaderBytecode.Get());
    if (success != S_OK)
    {
      LOG_ERROR("Failed to load the compiled shader bytecode into the dxc reflector");
      return false;
    }

    ShaderCompiledDataDX12 compiledNativeData;

    uint rootSigPartIdx;
    ASSERT(S_OK != dxcReflection->FindFirstPartKind(DXC_PART_ROOT_SIGNATURE, &rootSigPartIdx), "Custom HLSL-specified root signatures are not supported");

    if (!IsRaytracingStage(static_cast<ShaderStage>(aDesc.myShaderStage)))  // Raytracing-shaders are always compiled as library shaders and can't be reflected
    {
      // Shader reflection
      //---------------------------------------------------------------------------//
        uint dxilPartIdx;
        success = dxcReflection->FindFirstPartKind(DXC_PART_DXIL, &dxilPartIdx);
        ASSERT(success == S_OK);

        Microsoft::WRL::ComPtr<ID3D12ShaderReflection> reflector;
        success = dxcReflection->GetPartReflection(dxilPartIdx, IID_PPV_ARGS(&reflector));
        if (S_OK != success)
        {
          LOG_ERROR("Failed reflecting shader");
          return false;
        }

        if (aDesc.myShaderStage == static_cast<uint>(ShaderStage::SHADERSTAGE_VERTEX))
        {
          D3D12_SHADER_DESC shaderDesc;
          reflector->GetDesc(&shaderDesc);

          eastl::fixed_vector<VertexShaderAttributeDesc, 16>& vertexAttributes = anOutput->myVertexAttributes;
          for (uint i = 0u; i < shaderDesc.InputParameters; ++i)
          {
            D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
            reflector->GetInputParameterDesc(i, &paramDesc);

            VertexShaderAttributeDesc& attributeDesc = vertexAttributes.push_back();
            attributeDesc.myFormat = locResolveFormat(paramDesc);
            ASSERT(attributeDesc.myFormat != DataFormat::NONE);
            attributeDesc.mySemantic = ShaderCompiler::GetVertexAttributeSemantic(paramDesc.SemanticName);
            attributeDesc.mySemanticIndex = paramDesc.SemanticIndex;
          }

          // Create a default vertex input layout that assumes that all vertex attributes come from one interleaved vertex buffer.
         // A custom vertex input layout can be set using using CommandList::SetVertexInputLayout()
          uint overallVertexSize = 0u;
          VertexInputLayoutProperties props;
          for (uint i = 0u; i < vertexAttributes.size(); ++i)
          {
            const VertexShaderAttributeDesc& shaderAttribute = vertexAttributes[i];
            props.myAttributes.push_back({ shaderAttribute.myFormat, shaderAttribute.mySemantic, shaderAttribute.mySemanticIndex, 0u });
            overallVertexSize += BITS_TO_BYTES(DataFormatInfo::GetFormatInfo(shaderAttribute.myFormat).myBitsPerPixel);
          }

          props.myBufferBindings.push_back({ overallVertexSize, VertexInputRate::PER_VERTEX });
          anOutput->myDefaultVertexInputLayout = RenderCore::CreateVertexInputLayout(props);
        }
        else if (aDesc.myShaderStage == static_cast<uint>(ShaderStage::SHADERSTAGE_COMPUTE))
        {
          uint x, y, z;
          reflector->GetThreadGroupSize(&x, &y, &z);
          anOutput->myProperties.myNumGroupThreads = glm::int3(static_cast<int>(x), static_cast<int>(y), static_cast<int>(z));
        }
    }
    
    compiledNativeData.myBytecode.resize(compiledShaderBytecode->GetBufferSize());
    memcpy(compiledNativeData.myBytecode.data(), compiledShaderBytecode->GetBufferPointer(), compiledShaderBytecode->GetBufferSize());

    anOutput->myDx12Data = compiledNativeData;
    anOutput->myIncludedFilePaths = includeInfo.myIncludedFiles;

    return true;
  }
//---------------------------------------------------------------------------//
}

#endif