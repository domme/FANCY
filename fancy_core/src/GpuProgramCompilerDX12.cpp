#include "StdAfx.h"
#include "GpuProgramCompilerUtils.h"
#include "StringUtil.h"
#include "GpuProgramFeatures.h"
#include "GpuProgram.h"
#include "PathService.h"
#include "ShaderResourceInterface.h"
#include "Fancy.h"

#if defined (RENDERER_DX12)

#include "GpuProgramCompilerDX12.h"
#include "RendererDX12.h"
#include "Renderer.h"
#include "Log.h"

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
        ASSERT(false, "Unsupported HLSL shader-profile");
        return "";
        break;
    }
  }
//---------------------------------------------------------------------------//
  /*
  DataFormat locResolveVectorFormat(const D3D12_SHADER_TYPE_DESC& aTypeDesc)
  {
    switch(aTypeDesc.Type)
    {
      case D3D_SVT_BOOL: 
        switch(aTypeDesc.Columns) {
          case 2: return DataFormat::RG_32UI;
          case 3: return DataFormat::RGB_32UI;
          case 4: return DataFormat::RGBA_32UI; 
        }
        break;
      case D3D_SVT_INT: 
        switch (aTypeDesc.Columns) {
        case 2: return DataFormat::RG_32I;
        case 3: return DataFormat::RGB_32I;
        case 4: return DataFormat::RGBA_32I;
        }
        break;
      case D3D_SVT_FLOAT:
        switch (aTypeDesc.Columns) {
        case 2: return DataFormat::RG_32F;
        case 3: return DataFormat::RGB_32I;
        case 4: return DataFormat::RGBA_32I;
        }
        break;
      case D3D_SVT_STRING: break;
      case D3D_SVT_TEXTURE: break;
      case D3D_SVT_TEXTURE1D: break;
      case D3D_SVT_TEXTURE2D: break;
      case D3D_SVT_TEXTURE3D: break;
      case D3D_SVT_TEXTURECUBE: break;
      case D3D_SVT_SAMPLER: break;
      case D3D_SVT_SAMPLER1D: break;
      case D3D_SVT_SAMPLER2D: break;
      case D3D_SVT_SAMPLER3D: break;
      case D3D_SVT_SAMPLERCUBE: break;
      case D3D_SVT_PIXELSHADER: break;
      case D3D_SVT_VERTEXSHADER: break;
      case D3D_SVT_PIXELFRAGMENT: break;
      case D3D_SVT_VERTEXFRAGMENT: break;
      case D3D_SVT_UINT: break;
      case D3D_SVT_UINT8: break;
      case D3D_SVT_GEOMETRYSHADER: break;
      case D3D_SVT_RASTERIZER: break;
      case D3D_SVT_DEPTHSTENCIL: break;
      case D3D_SVT_BLEND: break;
      case D3D_SVT_BUFFER: break;
      case D3D_SVT_CBUFFER: break;
      case D3D_SVT_TBUFFER: break;
      case D3D_SVT_TEXTURE1DARRAY: break;
      case D3D_SVT_TEXTURE2DARRAY: break;
      case D3D_SVT_RENDERTARGETVIEW: break;
      case D3D_SVT_DEPTHSTENCILVIEW: break;
      case D3D_SVT_TEXTURE2DMS: break;
      case D3D_SVT_TEXTURE2DMSARRAY: break;
      case D3D_SVT_TEXTURECUBEARRAY: break;
      case D3D_SVT_HULLSHADER: break;
      case D3D_SVT_DOMAINSHADER: break;
      case D3D_SVT_INTERFACE_POINTER: break;
      case D3D_SVT_COMPUTESHADER: break;
      case D3D_SVT_DOUBLE: break;
      case D3D_SVT_RWTEXTURE1D: break;
      case D3D_SVT_RWTEXTURE1DARRAY: break;
      case D3D_SVT_RWTEXTURE2D: break;
      case D3D_SVT_RWTEXTURE2DARRAY: break;
      case D3D_SVT_RWTEXTURE3D: break;
      case D3D_SVT_RWBUFFER: break;
      case D3D_SVT_BYTEADDRESS_BUFFER: break;
      case D3D_SVT_RWBYTEADDRESS_BUFFER: break;
      case D3D_SVT_STRUCTURED_BUFFER: break;
      case D3D_SVT_RWSTRUCTURED_BUFFER: break;
      case D3D_SVT_APPEND_STRUCTURED_BUFFER: break;
      case D3D_SVT_CONSUME_STRUCTURED_BUFFER: break;
      case D3D_SVT_MIN8FLOAT: break;
      case D3D_SVT_MIN10FLOAT: break;
      case D3D_SVT_MIN16FLOAT: break;
      case D3D_SVT_MIN12INT: break;
      case D3D_SVT_MIN16INT: break;
      case D3D_SVT_MIN16UINT: break;
      case D3D10_SVT_VOID: break;
      case D3D10_SVT_BOOL: break;
      case D3D10_SVT_INT: break;
      case D3D10_SVT_FLOAT: break;
      case D3D10_SVT_STRING: break;
      case D3D10_SVT_TEXTURE: break;
      case D3D10_SVT_TEXTURE1D: break;
      case D3D10_SVT_TEXTURE2D: break;
      case D3D10_SVT_TEXTURE3D: break;
      case D3D10_SVT_TEXTURECUBE: break;
      case D3D10_SVT_SAMPLER: break;
      case D3D10_SVT_SAMPLER1D: break;
      case D3D10_SVT_SAMPLER2D: break;
      case D3D10_SVT_SAMPLER3D: break;
      case D3D10_SVT_SAMPLERCUBE: break;
      case D3D10_SVT_PIXELSHADER: break;
      case D3D10_SVT_VERTEXSHADER: break;
      case D3D10_SVT_PIXELFRAGMENT: break;
      case D3D10_SVT_VERTEXFRAGMENT: break;
      case D3D10_SVT_UINT: break;
      case D3D10_SVT_UINT8: break;
      case D3D10_SVT_GEOMETRYSHADER: break;
      case D3D10_SVT_RASTERIZER: break;
      case D3D10_SVT_DEPTHSTENCIL: break;
      case D3D10_SVT_BLEND: break;
      case D3D10_SVT_BUFFER: break;
      case D3D10_SVT_CBUFFER: break;
      case D3D10_SVT_TBUFFER: break;
      case D3D10_SVT_TEXTURE1DARRAY: break;
      case D3D10_SVT_TEXTURE2DARRAY: break;
      case D3D10_SVT_RENDERTARGETVIEW: break;
      case D3D10_SVT_DEPTHSTENCILVIEW: break;
      case D3D10_SVT_TEXTURE2DMS: break;
      case D3D10_SVT_TEXTURE2DMSARRAY: break;
      case D3D10_SVT_TEXTURECUBEARRAY: break;
      case D3D11_SVT_HULLSHADER: break;
      case D3D11_SVT_DOMAINSHADER: break;
      case D3D11_SVT_INTERFACE_POINTER: break;
      case D3D11_SVT_COMPUTESHADER: break;
      case D3D11_SVT_DOUBLE: break;
      case D3D11_SVT_RWTEXTURE1D: break;
      case D3D11_SVT_RWTEXTURE1DARRAY: break;
      case D3D11_SVT_RWTEXTURE2D: break;
      case D3D11_SVT_RWTEXTURE2DARRAY: break;
      case D3D11_SVT_RWTEXTURE3D: break;
      case D3D11_SVT_RWBUFFER: break;
      case D3D11_SVT_BYTEADDRESS_BUFFER: break;
      case D3D11_SVT_RWBYTEADDRESS_BUFFER: break;
      case D3D11_SVT_STRUCTURED_BUFFER: break;
      case D3D11_SVT_RWSTRUCTURED_BUFFER: break;
      case D3D11_SVT_APPEND_STRUCTURED_BUFFER: break;
      case D3D11_SVT_CONSUME_STRUCTURED_BUFFER: break;
      case D3D_SVT_FORCE_DWORD: break;
      default: break;
    }
  }
//---------------------------------------------------------------------------//
  DataFormat locResolveMatrixFormat(const D3D12_SHADER_TYPE_DESC& aTypeDesc)
  {
    
  }

  DataFormat locResolveScalarFormat(const D3D12_SHADER_TYPE_DESC& aTypeDesc)
  {
    
  }

//---------------------------------------------------------------------------//
  DataFormat locResolveFormat(const D3D12_SHADER_TYPE_DESC& aTypeDesc)
  {
    switch(aTypeDesc.Class)
    {
      case D3D_SVC_SCALAR:
        return locResolveScalarFormat(aTypeDesc);
        break;
      case D3D_SVC_VECTOR: 
        return locResolveVectorFormat(aTypeDesc);
        break;
      case D3D_SVC_MATRIX_ROWS: 
      case D3D_SVC_MATRIX_COLUMNS: 
        return locResolveMatrixFormat(aTypeDesc);
        break;
      case D3D_SVC_OBJECT: break;
      case D3D_SVC_STRUCT: break;
      case D3D_SVC_INTERFACE_CLASS: break;
      case D3D_SVC_INTERFACE_POINTER: break;
      default: break;
    }
  } */
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
  uint32 locResolveSizeBytes(const D3D12_SIGNATURE_PARAMETER_DESC& aParamDesc)
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
  uint32 locResolveComponentCount(const D3D12_SIGNATURE_PARAMETER_DESC& aParamDesc)
  {
    if (aParamDesc.Mask <= 1) return 1u;
    else if (aParamDesc.Mask <= 3) return 2u;
    else if (aParamDesc.Mask <= 7) return 3u;
    else if (aParamDesc.Mask <= 15) return 4u;

    ASSERT(false, "Unknown param-mask");
    return 0u;
  }
//---------------------------------------------------------------------------//
  void locResolveFormat(const char* aTypeName, uint32& aSizeBytesOut, DataFormat& aDataFormatOut)
  {
    uint32 sizeBytes = 0u;
    DataFormat format = DataFormat::NONE;

#define CHECK(str) strcmp(aTypeName, str) == 0
    
    const uint32 kFloatSize = 4u;
    const uint32 kUintSize = 4u;
    const uint32 kIntSize = 4u;

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

    else if (CHECK("uint"))
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
  bool locReflectConstants(ID3D12ShaderReflection* aReflector, const D3D12_SHADER_DESC& aShaderDesc, GpuProgramCompilerOutputDX12* aCompilerOutput)
  {
    aCompilerOutput->myConstantBufferElements.clear();

    for (uint32 i = 0u; i < aShaderDesc.ConstantBuffers; ++i)
    {
      ID3D12ShaderReflectionConstantBuffer* cb = aReflector->GetConstantBufferByIndex(i);
      
      D3D12_SHADER_BUFFER_DESC cbDesc;
      cb->GetDesc(&cbDesc);

      uint32 currOffsetInCbuffer = 0u;
      for (uint32 iCBvar = 0u; iCBvar < cbDesc.Variables; ++iCBvar)
      {
        ID3D12ShaderReflectionVariable* cbVar = cb->GetVariableByIndex(iCBvar);

        D3D12_SHADER_VARIABLE_DESC cbVarDesc;
        cbVar->GetDesc(&cbVarDesc);
        
        ID3D12ShaderReflectionType* cbVarType = cbVar->GetType();
        D3D12_SHADER_TYPE_DESC cbVarTypeDesc;
        cbVarType->GetDesc(&cbVarTypeDesc);

        currOffsetInCbuffer += cbVarDesc.StartOffset;

        if (cbVarTypeDesc.Members > 0u)  // This is a struct
        {
          for (uint32 iMember = 0u; iMember < cbVarTypeDesc.Members; ++iMember)
          {
            ID3D12ShaderReflectionType* cbVarMemberType = cbVarType->GetMemberTypeByIndex(iMember);
            D3D12_SHADER_TYPE_DESC cbVarMemberTypeDesc;
            cbVarMemberType->GetDesc(&cbVarMemberTypeDesc);

            ConstantBufferElement cbElem;
            cbElem.uOffsetBytes = currOffsetInCbuffer + cbVarMemberTypeDesc.Offset;
            cbElem.name = cbVarType->GetMemberTypeName(iMember);

            uint32 sizeBytes;
            DataFormat format;
            locResolveFormat(cbVarMemberTypeDesc.Name, sizeBytes, format);
            cbElem.eFormat = format;
            cbElem.uSizeBytes = sizeBytes;
            cbElem.uFormatComponentCount = cbVarMemberTypeDesc.Rows;  // Columns is already encoded in the format

            aCompilerOutput->myConstantBufferElements.push_back(cbElem);
          }
        }
        else
        {
          ConstantBufferElement cbElem;
          cbElem.uOffsetBytes = cbVarTypeDesc.Offset;
          cbElem.uSizeBytes = cbVarDesc.Size;

          uint32 sizeBytes;
          DataFormat format;
          locResolveFormat(cbVarTypeDesc.Name, sizeBytes, format);
          cbElem.eFormat = format;
          cbElem.uFormatComponentCount = cbVarTypeDesc.Rows;
          cbElem.name = cbVarDesc.Name;

          aCompilerOutput->myConstantBufferElements.push_back(cbElem);
        }
      }
    }
    return true;
  }
//---------------------------------------------------------------------------//
  bool locReflectVertexInputLayout(ID3D12ShaderReflection* aReflector, 
    const D3D12_SHADER_DESC& aShaderDesc, GpuProgramCompilerOutputDX12* aCompilerOutput)
  {
    if (aShaderDesc.InputParameters == 0u)
      return false;

    aCompilerOutput->clVertexInputLayout.clear();

    D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
    for (uint i = 0u; i < aShaderDesc.InputParameters; ++i)
    {
      aReflector->GetInputParameterDesc(i, &paramDesc);
      
      ShaderVertexInputElement inputElem;

      inputElem.mySemantics = GpuProgramDX12::GetVertexSemanticFromShaderString(paramDesc.SemanticName);
      inputElem.mySemanticIndex = paramDesc.SemanticIndex;
      inputElem.myRegisterIndex = paramDesc.Register;
      inputElem.myFormat = locResolveFormat(paramDesc);
      inputElem.mySizeBytes = locResolveSizeBytes(paramDesc);
      inputElem.myName = paramDesc.SemanticName;
      inputElem.myFormatComponentCount = locResolveComponentCount(paramDesc);

      aCompilerOutput->clVertexInputLayout.addVertexInputElement(inputElem);
    }

    return true;
  }
//---------------------------------------------------------------------------//
  bool GpuProgramCompilerDX12::Compile(const GpuProgramDesc& aDesc, GpuProgramCompilerOutputDX12* aProgram)
  {
    LOG_INFO("Compiling shader %...", aDesc.myShaderPath);

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

    ComPtr<ID3DBlob> compiledShaderBytecode;
    ComPtr<ID3DBlob> errorData;

    HRESULT sucess = D3DCompileFromFile(
      shaderPathAbs.c_str(),
      &defines[0],
      D3D_COMPILE_STANDARD_FILE_INCLUDE,
      "main",
      shaderProfileStr.c_str(),
      D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_OPTIMIZATION_LEVEL0 | D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR,
      0u,
      &compiledShaderBytecode,
      &errorData);

    if (S_OK != sucess)
    {
      if (errorData != nullptr)
      {
        const char* errorMsg = (const char*)errorData->GetBufferPointer();
        LOG_ERROR(errorMsg);
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
    
    ID3D12Device* d3dDevice = Fancy::GetRenderer()->GetDevice();

    ID3D12RootSignature* rootSignature;
    sucess = d3dDevice->CreateRootSignature(0u, rsBlob->GetBufferPointer(), rsBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

    if (S_OK != sucess)
    {
      LOG_ERROR("Failed creating the root signature from shader");
      return false;
    }

    ComPtr<ID3D12RootSignatureDeserializer> rsDeserializer;
    sucess = D3D12CreateRootSignatureDeserializer(rsBlob->GetBufferPointer(), rsBlob->GetBufferSize(), IID_PPV_ARGS(&rsDeserializer));

    if (S_OK != sucess)
    {
      LOG_ERROR("Failed creating a rootSignature deserializer");
      return false;
    }

    const D3D12_ROOT_SIGNATURE_DESC* rsDesc = rsDeserializer->GetRootSignatureDesc();
    ShaderResourceInterface* rsObject = RenderCoreDX12::GetShaderResourceInterface(*rsDesc, rootSignature);
    ASSERT(nullptr != rsObject);

    aProgram->myPermutation = aDesc.myPermutation;
    aProgram->eShaderStage = static_cast<ShaderStage>(aDesc.myShaderStage);
    aProgram->myShaderFilename = aDesc.myShaderPath;
    aProgram->myRootSignature = rsObject;
    aProgram->myNativeData = compiledShaderBytecode;

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

    if (!locReflectConstants(reflector, shaderDesc, aProgram))
    {
      LOG_ERROR("Failed reflecting constants");
      return false;
    }

    if (aDesc.myShaderStage == static_cast<uint32>(ShaderStage::VERTEX))
    {
      if (!locReflectVertexInputLayout(reflector, shaderDesc, aProgram))
      {
        LOG_ERROR("Failed reflecting vertex input layout");
        return false;
      }
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
