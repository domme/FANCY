#include "fancy_core_precompile.h"

#if FANCY_ENABLE_VK

#include "IO/FileReader.h"

#include "Rendering/RenderCore.h"

#include "ShaderCompilerVk.h"

#include "VkPrerequisites.h"
#include "ShaderVk.h"

#include "RenderCore_PlatformVk.h"
#include "spirv_reflect.h"

namespace Fancy
{
  namespace Priv_ShaderCompilerVk
  {
//---------------------------------------------------------------------------//
    DataFormat locResolveFormat(SpvReflectFormat aFormat)
    {
      switch(aFormat) 
      { 
        case SPV_REFLECT_FORMAT_UNDEFINED: return DataFormat::NONE;
        case SPV_REFLECT_FORMAT_R32_UINT: return DataFormat::R_32UI;
        case SPV_REFLECT_FORMAT_R32_SINT: return DataFormat::R_32I;
        case SPV_REFLECT_FORMAT_R32_SFLOAT: return DataFormat::R_32F;
        case SPV_REFLECT_FORMAT_R32G32_UINT: return DataFormat::RG_32UI;
        case SPV_REFLECT_FORMAT_R32G32_SINT: return DataFormat::RG_32I;
        case SPV_REFLECT_FORMAT_R32G32_SFLOAT: return DataFormat::RG_32F;
        case SPV_REFLECT_FORMAT_R32G32B32_UINT: return DataFormat::RGB_32UI;
        case SPV_REFLECT_FORMAT_R32G32B32_SINT: return DataFormat::RGB_32I;
        case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT: return DataFormat::RGB_32F;
        case SPV_REFLECT_FORMAT_R32G32B32A32_UINT: return DataFormat::RGBA_32UI;
        case SPV_REFLECT_FORMAT_R32G32B32A32_SINT: return DataFormat::RGBA_32I;
        case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT: return DataFormat::RGBA_32F;
        default: ASSERT(false, "Missing implementation"); return DataFormat::NONE;
      }
    }
//---------------------------------------------------------------------------//
    void locResolveSemantic(const char* aSemanticString, VertexAttributeSemantic& aSemanticOut, uint& aSemanticIndexOut)
    {
      const int strLen = (int)strlen(aSemanticString);
      int indexStartPos = -1;
      for (int i = strLen - 1; i > 0; --i)
      {
        const char c = aSemanticString[i];
        if (c >= '0' && c <= '9')
          indexStartPos = i;
        else
          break;
      }

      if (indexStartPos == -1)
      {
        aSemanticIndexOut = 0u;
      }
      else
      {
        const int numIndexChars = strLen - indexStartPos;

        char buf[16];
        ASSERT((int) ARRAY_LENGTH(buf) > numIndexChars);
        memcpy(buf, &aSemanticString[indexStartPos], sizeof(char) * numIndexChars);
        buf[numIndexChars] = '\0';

        aSemanticIndexOut = (uint)atoi(buf);
      }
  
      aSemanticOut = ShaderCompiler::GetVertexAttributeSemantic(aSemanticString);
    }
//---------------------------------------------------------------------------//
  }
//---------------------------------------------------------------------------//
  bool ShaderCompilerVk::Compile_Internal(const char* anHlslSrcPathAbs, const ShaderDesc& aDesc, ShaderCompilerResult* aCompilerOutput) const
  {
    // TODO: Add fast-path if the cache-file is newer than the src-file and directly load the SPV binary from that file
    // const uint64 srcFileWriteTime = Path::GetFileWriteTime(hlslSrcPathAbs);

    DxcShaderCompiler::Config config = 
    {
      true,
      true,
      GetHLSLprofileString((ShaderStage) aDesc.myShaderStage).c_str()
    };

    DxcShaderCompiler::IncludeInfo includeInfo;
    eastl::vector<uint8> spvBinaryData;
    if (!myDxcCompiler.CompileToBytecode(anHlslSrcPathAbs, aDesc, config, includeInfo, spvBinaryData))
      return false;

    // TODO: Save shader-cache?
    // const uint64 shaderHash = aDesc.GetHash();
    // String spvBinaryFilePathAbs(StaticFilePath("%sShaderCache/%llu.spv", Path::GetUserDataPath().c_str(), shaderHash));
    // Path::CreateDirectoryTreeForPath(spvBinaryFilePathAbs);

    VkShaderModuleCreateInfo moduleCreateInfo = {};
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.codeSize = spvBinaryData.size();
    moduleCreateInfo.pCode = reinterpret_cast<uint32_t*>(spvBinaryData.data());

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

    ShaderCompiledDataVk compiledDataVk;
    ASSERT_VK_RESULT(vkCreateShaderModule(platformVk->myDevice, &moduleCreateInfo, nullptr, &compiledDataVk.myModule));

    compiledDataVk.myBytecodeHash = MathUtil::ByteHash(spvBinaryData.data(), spvBinaryData.size());
    
    // Reflect the spirv data
    SpvReflectShaderModule reflectModule;
    SpvReflectResult reflectResult = spvReflectCreateShaderModule(spvBinaryData.size(), spvBinaryData.data(), &reflectModule);
    ASSERT(reflectResult == SPV_REFLECT_RESULT_SUCCESS);

    if (aDesc.myShaderStage == (uint) ShaderStage::SHADERSTAGE_VERTEX)
    {
      eastl::fixed_vector<VertexShaderAttributeDesc, 16>& vertexAttributes = aCompilerOutput->myVertexAttributes;
      eastl::fixed_vector<uint, 16>& vertexAttributeLocations = compiledDataVk.myVertexAttributeLocations;
      
      for (uint i = 0u; i < reflectModule.input_variable_count; ++i)
      {
        const SpvReflectInterfaceVariable* reflectedInput = reflectModule.input_variables[i];

        const DataFormat format = Priv_ShaderCompilerVk::locResolveFormat(reflectedInput->format);
        ASSERT(format != DataFormat::NONE);

        const char* semanticStringBuf = reflectedInput->semantic;
        eastl::string semanticString;
        if (semanticStringBuf == nullptr)  // should be emitted by -fspv-reflect but that requires SPV_GOOGLE_HLSL_Functionality1 which is not available on NVIDIA
        {
          // The name should be formed like this: "in.var.POSITION", so we should be able to just use the last part of that name as the semantic (hopefully..)
          semanticString = reflectedInput->name;
          size_t pos = semanticString.find_last_of('.');
          ASSERT(pos != eastl::string::npos);

          semanticString.erase(0, pos + 1);
          semanticStringBuf = semanticString.data();
        }
        
        VertexAttributeSemantic semantic;
        uint semanticIndex;
        Priv_ShaderCompilerVk::locResolveSemantic(semanticStringBuf, semantic, semanticIndex);
        
        vertexAttributes.push_back({ semantic, semanticIndex, format});
        vertexAttributeLocations.push_back(reflectedInput->location);
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
      aCompilerOutput->myDefaultVertexInputLayout = RenderCore::CreateVertexInputLayout(props);
    }
    else if (aDesc.myShaderStage == (uint)ShaderStage::SHADERSTAGE_COMPUTE)
    {
      // SPIR-V reflect doesn't provide a way to reflect group-thread count yet, so just parse the source-code as a workaround for now
      eastl::string hlslSource = FileReader::ReadTextFile(anHlslSrcPathAbs);
      ASSERT(hlslSource.size() > 0);

      size_t mainFuncPos = hlslSource.find("void " + aDesc.myMainFunction);
      ASSERT(mainFuncPos != eastl::string::npos);

      const char* numthreadsSearchKey = "[numthreads(";
      size_t numThreadsPos = hlslSource.rfind(numthreadsSearchKey, mainFuncPos);
      ASSERT(numThreadsPos != eastl::string::npos);

      char delims[3] = { ',', ',', ')' };
      int numGroupThreads[3] = { -1, -1, -1 };
      
      int i = (int)numThreadsPos + (int) strlen(numthreadsSearchKey);
      for (int cat = 0; cat < 3; ++cat)
      {
        int numChars = 0;
        char buf[16];
        for ( ; hlslSource[i] != delims[cat] && i < (int) hlslSource.size(); ++i)
        {
          const char c = hlslSource[i];
          if (c != ' ')
            buf[numChars++] = c;
        }
        ++i;
        buf[numChars] = '\0';
        numGroupThreads[cat] = atoi(buf);
        ASSERT(numGroupThreads[cat] != -1);
      }

      aCompilerOutput->myProperties.myNumGroupThreads = glm::int3(numGroupThreads[0], numGroupThreads[1], numGroupThreads[2]);
    }

    spvReflectDestroyShaderModule(&reflectModule);

    aCompilerOutput->myNativeData = compiledDataVk;
    aCompilerOutput->myIncludedFilePaths = eastl::move(includeInfo.myIncludedFiles);

    return true;
  }
//---------------------------------------------------------------------------//
}

#endif