#include "fancy_core_precompile.h"
#include "ShaderCompilerVk.h"
#include "PathService.h"
#include "FileReader.h"
#include "VkPrerequisites.h"
#include "ShaderVk.h"
#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"
#include "StaticArray.h"
#include "PathService.h"

#include "spirv_reflect/spirv_reflect.h"

#include <dxc/dxcapi.h>
#include <atomic>

namespace Fancy
{
  namespace Priv_ShaderCompilerVk
  {
//---------------------------------------------------------------------------//
    StaticFilePath locGetShaderPath(const char* aFilename, bool& isGlsl)
    {
      StaticFilePath path("%s/Vk/%s.glsl", ShaderCompiler::GetShaderRootFolderRelative(), aFilename);

      String pathRel = path.GetBuffer();
      String pathAbs = Resources::FindPath(pathRel);

      if (Path::FileExists(pathAbs.c_str()))
      {
        isGlsl = true;
        return path;
      }

      isGlsl = false;
      path.Format("%s/DX12/%s.hlsl", ShaderCompiler::GetShaderRootFolderRelative(), aFilename);
      return path;
    }
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
    void locResolveSemantic(const char* aSemanticString, VertexSemantics& aSemanticOut, uint& aSemanticIndexOut)
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
  
      if (strncmp(aSemanticString, "POSITION", strlen("POSITION")) == 0)
        aSemanticOut = VertexSemantics::POSITION;
      else if (strncmp(aSemanticString, "NORMAL", strlen("NORMAL")) == 0)
        aSemanticOut = VertexSemantics::NORMAL;
      else if (strncmp(aSemanticString, "TANGENT", strlen("TANGENT")) == 0)
        aSemanticOut = VertexSemantics::TANGENT;
      else if (strncmp(aSemanticString, "BINORMAL", strlen("BINORMAL")) == 0)
        aSemanticOut = VertexSemantics::BITANGENT;
      else if (strncmp(aSemanticString, "TEXCOORD", strlen("TEXCOORD")) == 0)
        aSemanticOut = VertexSemantics::TEXCOORD;
      else if (strncmp(aSemanticString, "COLOR", strlen("COLOR")) == 0)
        aSemanticOut = VertexSemantics::COLOR;
      else
        ASSERT(false, "Unrecognized vertex semantic %s", aSemanticString);
    }
//---------------------------------------------------------------------------//
    VkDescriptorType locResolveDescriptorType(SpvReflectDescriptorType aReflDescType)
    {
      switch (aReflDescType)
      {
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER: return VK_DESCRIPTOR_TYPE_SAMPLER;
        case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        default: ASSERT(false, "Missing implementation"); return VK_DESCRIPTOR_TYPE_SAMPLER;
      }
    }
//---------------------------------------------------------------------------//
    struct IncludeHandler : IDxcIncludeHandler
    {
      StaticArray<std::wstring, 16> myIncludeSearchPaths;
      IDxcLibrary* myDxcLibrary;

      IncludeHandler(IDxcLibrary* aDxcLibrary, String* someIncludeSearchPaths, uint aNumPaths)
        : myDxcLibrary(aDxcLibrary)
      {
        for (uint i = 0u; i < aNumPaths; ++i)
        {
          const String& dir = someIncludeSearchPaths[i];
          ASSERT(!dir.empty());

          if (dir[dir.size() - 1] != '/' && dir[dir.size() - 1] != '\\')
            myIncludeSearchPaths.Add(StringUtil::ToWideString(dir + '/'));
          else
            myIncludeSearchPaths.Add(StringUtil::ToWideString(dir));
        }
          
      }

      HRESULT LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource) override
      {
        // Skip current-folder markers "./" 
        if (wcslen(pFilename) > 1 && pFilename[0] == '.' && (pFilename[1] == '/' || pFilename[1] == '\\'))
          pFilename += 2;

        if (Path::FileExists(pFilename))
        {
          IDxcBlobEncoding* pBlobWithEncoding;
          HRESULT result = myDxcLibrary->CreateBlobFromFile(pFilename, nullptr, &pBlobWithEncoding);
          ASSERT(result == S_OK);
          *ppIncludeSource = pBlobWithEncoding;
          return S_OK;
        }

        for (uint i = 0u; i < myIncludeSearchPaths.Size(); ++i)
        {
          std::wstring path = myIncludeSearchPaths[i] + pFilename;
          if (Path::FileExists(path.c_str()))
          {
            IDxcBlobEncoding* pBlobWithEncoding;
            HRESULT result = myDxcLibrary->CreateBlobFromFile(path.c_str(), nullptr, &pBlobWithEncoding);
            ASSERT(result == S_OK);
            *ppIncludeSource = pBlobWithEncoding;
            return S_OK;
          }
        }

        return E_FAIL;
      }

      HRESULT QueryInterface(const IID& riid, void** ppvObject) override
      {
        if (!ppvObject)
          return E_INVALIDARG;
        *ppvObject = nullptr;

        if (riid == IID_IUnknown || riid == __uuidof(IDxcIncludeHandler))
        {
          *ppvObject = this;
          AddRef();
          return NOERROR;
        }
        return E_NOINTERFACE;
      }

      ULONG AddRef() override
      {
        myRefCount++;
        return (ULONG) myRefCount;
      }

      ULONG Release() override
      {
        int refCount = --myRefCount;
        if (myRefCount == 0)
        {
          delete this;
        }
        return (ULONG) refCount;
      }

      std::atomic<int> myRefCount = 1;
    };
//---------------------------------------------------------------------------//
  }
//---------------------------------------------------------------------------//
  ShaderCompilerVk::ShaderCompilerVk()
  {
    HRESULT hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&myDxcLibrary));
    ASSERT(hr == S_OK, "Failed to create DXC library");
    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&myDxcCompiler));
    ASSERT(hr == S_OK, "Failed to create DXC compiler");
  }
//---------------------------------------------------------------------------//
  ShaderCompilerVk::~ShaderCompilerVk()
  {
  }
//---------------------------------------------------------------------------//
  String ShaderCompilerVk::GetShaderPath(const char* aFilename) const
  {
    bool isGlsl = false;
    return Priv_ShaderCompilerVk::locGetShaderPath(aFilename, isGlsl).GetBuffer();
  }
//---------------------------------------------------------------------------//
  bool ShaderCompilerVk::Compile_Internal(const ShaderDesc& aDesc, const char* aStageDefine, ShaderCompilerResult* aCompilerOutput) const
  {
    // The Vulkan shader compiler should work like that:
    // - Translate default HLSL shader into a SPIR-V binary
    // - If there is a .../Vulkan/.glsl-file for that shader as well (or only that) prefer that one over the HLSL file. That way, HLSL-translation problems can be worked around by providing a glsl shader
    // - In the future: Use caches to skip source-file compilation!

    bool isGlsl = false;
    String hlslSrcPathAbs = Priv_ShaderCompilerVk::locGetShaderPath(aDesc.myShaderFileName.c_str(), isGlsl).GetBuffer();
    bool fileFound = false;

    // TODO: Add a glsl->SPIR-V compilation in case there is a glsl shader that should be picked over the HLSL one

    hlslSrcPathAbs = Resources::FindPath(hlslSrcPathAbs, &fileFound);
    if (!fileFound)
      return false;

    // TODO: Add fast-path if the cache-file is newer than the src-file and directly load the SPV binary from that file
    // const uint64 srcFileWriteTime = Path::GetFileWriteTime(hlslSrcPathAbs);

    const uint64 shaderHash = aDesc.GetHash();
    String spvBinaryFilePathAbs(StaticFilePath("%sShaderCache/%llu.spv", Path::GetUserDataPath().c_str(), shaderHash));
    Path::CreateDirectoryTreeForPath(spvBinaryFilePathAbs);

    DynamicArray<uint8> spvBinaryData;
    if (!isGlsl)
    {
      std::string shaderFile = FileReader::ReadTextFile(hlslSrcPathAbs.c_str());
      if (shaderFile.empty())
        return false;
      
      IDxcBlobEncoding* sourceBlob;
      if (myDxcLibrary->CreateBlobWithEncodingFromPinned(shaderFile.c_str(), (uint) shaderFile.size(), CP_UTF8, &sourceBlob) != S_OK)
        return false;

      LPCWSTR args[32];
      uint numArgs = 0u;

      auto AddArgument = [&](LPCWSTR anArg)
      {
        ASSERT(numArgs < ARRAY_LENGTH(args));
        args[numArgs++] = anArg;
      };
      
      AddArgument(L"/spirv");                 // Generate SPIR-V code
      AddArgument(L"/fspv-reflect");          // Emit additional SPIR-V instructions to aid reflection
      AddArgument(L"/fvk-use-dx-layout");     // Use DirectX memory layout for Vulkan resources
      AddArgument(L"/fvk-use-dx-position-w"); // Reciprocate SV_Position.w after reading from stage input in PS to accommodate the difference between Vulkan and DirectX
      AddArgument(L"/Zpc");                   // Pack matrices in column-major order
      AddArgument(L"/Zi");                    // Enable debug information
      AddArgument(L"/Qembed_debug");          // Silence warning about embedding PDBs into the shader container
      if (aDesc.myShaderStage == (uint)ShaderStage::VERTEX)
        AddArgument(L"/fvk-invert-y");

      StaticArray<std::wstring, 32> defineNames;
      StaticArray<DxcDefine, 32> defines;
      for (const String& define : aDesc.myDefines)
      {
        defineNames.Add(StringUtil::ToWideString(define));
        defines.Add({ defineNames[defineNames.Size() - 1].c_str(), nullptr });
      }

      defineNames.Add(L"DXC_COMPILER");
      defines.Add({ defineNames[defineNames.Size() - 1].c_str(), nullptr });

      defineNames.Add(StringUtil::ToWideString(aStageDefine));
      defines.Add({ defineNames[defineNames.Size() - 1].c_str(), nullptr });

      String includePaths[] =
      {
        Path::GetContainingFolder(hlslSrcPathAbs),
        Path::GetAbsolutePath(GetShaderRootFolderRelative()),
        Path::GetAbsolutePath(String(GetShaderRootFolderRelative()) + "/DX12"),
      };
      Microsoft::WRL::ComPtr<Priv_ShaderCompilerVk::IncludeHandler> includeHandler = 
        new Priv_ShaderCompilerVk::IncludeHandler(myDxcLibrary.Get(), includePaths, ARRAY_LENGTH(includePaths));

      IDxcOperationResult* compiledResult;
      HRESULT result = myDxcCompiler->Compile(
        sourceBlob,
        StringUtil::ToWideString(aDesc.myShaderFileName).c_str(),
        StringUtil::ToWideString(aDesc.myMainFunction).c_str(),
        StringUtil::ToWideString(GetHLSLprofileString(static_cast<ShaderStage>(aDesc.myShaderStage))).c_str(),
        args,
        numArgs,
        defines.GetBuffer(),
        defines.Size(),
        includeHandler.Get(),
        &compiledResult);

      IDxcBlobEncoding* errorBlob = nullptr;
      compiledResult->GetErrorBuffer(&errorBlob);

      if (errorBlob != nullptr)
      {
        IDxcBlobEncoding* errorBlob8 = nullptr;
        myDxcLibrary->GetBlobAsUtf8(errorBlob, &errorBlob8);

        if (errorBlob8 != nullptr &&  static_cast<const char*>(errorBlob8->GetBufferPointer())[0] != '\0')
        {
          LOG_ERROR("Error compiling shader %s: %s", aDesc.myShaderFileName.c_str(), static_cast<const char*>(errorBlob8->GetBufferPointer()));
          return false;
        }
      }

      IDxcBlob* spirvBlob;
      result = compiledResult->GetResult(&spirvBlob);
      if (result != S_OK)
      {
        LOG_ERROR("Failed getting compiled binary result of shader %s", aDesc.myShaderFileName.c_str());
        return false;
      }

      spvBinaryData.resize(spirvBlob->GetBufferSize());
      memcpy(spvBinaryData.data(), spirvBlob->GetBufferPointer(), spirvBlob->GetBufferSize());

    /*
    String dxcPath = Path::GetAppPath() + "/../../../dependencies/bin/dxc.exe";
    Path::RemoveFolderUpMarkers(dxcPath);
    
    if (!isGlsl)  // Use dxc.exe to convert from HLSL to SPIR-V binary
    {
      String commandStr = Path::GetAsCmdParameter(dxcPath.c_str()) + " " +
        "-spirv "  // Generate SPIR-V code
        "-fspv-reflect "  // Emit additional SPIR-V instructions to aid reflection
        "-fvk-use-dx-layout "  // Use DirectX memory layout for Vulkan resources
        "-fvk-use-dx-position-w " // Reciprocate SV_Position.w after reading from stage input in PS to accommodate the difference between Vulkan and DirectX
        // "-o0 " // Optimization Level 0  (Seems to be not recognized?!)
        "-Zpc " // Pack matrices in column-major order
        "-Zi " // Enable debug information
        + "-E " + aDesc.myMainFunction + " "
        + "-T " + GetHLSLprofileString(static_cast<ShaderStage>(aDesc.myShaderStage)) + " "
        + "-D " + aStageDefine + " "
        + "-D " + "DXC_COMPILER ";

      for (const String& define : aDesc.myDefines)
        commandStr += "-D " + define + " ";

      if (aDesc.myShaderStage == (uint)ShaderStage::VERTEX)
        commandStr += "-fvk-invert-y ";  // Negate SV_Position.y before writing to stage output in VS/DS/GS to accommodate Vulkan's coordinate system

      // Add include search paths
      String includeSearchFolders[] = {
        Path::GetContainingFolder(hlslSrcPathAbs),
        Path::GetAbsolutePath(GetShaderRootFolderRelative()),
        Path::GetAbsolutePath(String(GetShaderRootFolderRelative()) + "/DX12"),
      };
      for (String& include : includeSearchFolders)
      {
        // For some reason dxc only understands include-paths in escaped backslash-format
        std::replace(include.begin(), include.end(), '/', '\\');
        commandStr += "-I \"" + include + "\" ";
      }

      // Redirect output of command into file
      String errorOut(StaticFilePath("%sShaderCache/%llu_error.txt", Path::GetUserDataPath().c_str(), shaderHash));
      commandStr += "-Fe " + Path::GetAsCmdParameter(errorOut.c_str()) + " ";
      commandStr += Path::GetAsCmdParameter(hlslSrcPathAbs.c_str()) + " -Fo " + Path::GetAsCmdParameter(spvBinaryFilePathAbs.c_str());

      const int returnCode = system(commandStr.c_str());
      if (returnCode != 0)
      {
        String errorOutMsg = FileReader::ReadTextFile(errorOut.c_str());
        LOG_ERROR("Error compiling hlsl shader %s to SPIR-V: %s", hlslSrcPathAbs.c_str(), errorOutMsg.c_str());
        return false;
      }
      
      DynamicArray<uint8> spvBinaryData;
      const bool spvReadSuccess = FileReader::ReadBinaryFile(spvBinaryFilePathAbs.c_str(), spvBinaryData);
      ASSERT(spvReadSuccess);
      */

      VkShaderModuleCreateInfo moduleCreateInfo = {};
      moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      moduleCreateInfo.codeSize = spvBinaryData.size();
      moduleCreateInfo.pCode = reinterpret_cast<uint32_t*>(spvBinaryData.data());

      RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

      ShaderCompiledDataVk compiledDataVk;
      ASSERT_VK_RESULT(vkCreateShaderModule(platformVk->myDevice, &moduleCreateInfo, nullptr, &compiledDataVk.myModule));
      
      // Reflect the spirv data

      SpvReflectShaderModule reflectModule;
      SpvReflectResult reflectResult = spvReflectCreateShaderModule(spvBinaryData.size(), spvBinaryData.data(), &reflectModule);
      ASSERT(reflectResult == SPV_REFLECT_RESULT_SUCCESS);

      // Handle unordered writes. It seems we can only determine if a UAV is used in the shader but not if its actually written to in the shader.
      bool hasUnorderedWrites = false;
      for (uint i = 0u; i < reflectModule.descriptor_binding_count && !hasUnorderedWrites; ++i)
        hasUnorderedWrites |= (reflectModule.descriptor_bindings[i].resource_type & SpvReflectResourceType::SPV_REFLECT_RESOURCE_FLAG_UAV) != 0;
      aCompilerOutput->myProperties.myHasUnorderedWrites = hasUnorderedWrites;

      ShaderBindingInfoVk& bindingInfo = compiledDataVk.myBindingInfo;
      bindingInfo.myDescriptorSets.resize(reflectModule.descriptor_set_count);

      for (uint s = 0u; s < reflectModule.descriptor_set_count; ++s)
      {
        ShaderDescriptorSetBindingInfoVk& setInfo = bindingInfo.myDescriptorSets[s];
        const SpvReflectDescriptorSet& reflSet = reflectModule.descriptor_sets[s];

        setInfo.mySet = reflSet.set;
        setInfo.myBindings.resize(reflSet.binding_count);

        for (uint b = 0u; b < reflSet.binding_count; ++b )
        {
          const SpvReflectDescriptorBinding* reflBinding = reflSet.bindings[b];
          ASSERT(reflBinding != nullptr);

          ShaderDescriptorBindingVk& setBinding = setInfo.myBindings[b];
          setBinding.myBinding = reflBinding->binding;
          setBinding.myDescriptorCount = reflBinding->count;
          setBinding.myDescriptorType = Priv_ShaderCompilerVk::locResolveDescriptorType(reflBinding->descriptor_type);
        }
      }

      // Build the vertex input layout in case of vertex-shader
      if (aDesc.myShaderStage == (uint) ShaderStage::VERTEX)
      {
        ShaderVertexInputLayout& vertexInputlayout = aCompilerOutput->myProperties.myVertexInputLayout;
        vertexInputlayout.myVertexInputElements.reserve(reflectModule.input_variable_count);

        DynamicArray<VkVertexInputAttributeDescription>& vertexAttributes = compiledDataVk.myVertexAttributeDesc.myVertexAttributes;
        vertexAttributes.reserve(reflectModule.input_variable_count);

        // TODO: For simplicity, the code below assumes that all vertex attributes will be pulled from only one binding buffer in interleaved format
        uint overallVertexSize = 0u;
        for (uint i = 0u; i < reflectModule.input_variable_count; ++i)
        {
          const SpvReflectInterfaceVariable& reflectedInput = reflectModule.input_variables[i];

          const DataFormat format = Priv_ShaderCompilerVk::locResolveFormat(reflectedInput.format);
          ASSERT(format != DataFormat::NONE);

          const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(format);

          VertexSemantics semantic;
          uint semanticIndex;
          Priv_ShaderCompilerVk::locResolveSemantic(reflectedInput.semantic, semantic, semanticIndex);

          ShaderVertexInputElement elem =
          {
            reflectedInput.name,
            semantic,
            semanticIndex,
            formatInfo.mySizeBytes,
            format,
            (uint8) formatInfo.myNumComponents
          };
          vertexInputlayout.myVertexInputElements.push_back(elem);

          VkVertexInputAttributeDescription attribute;
          attribute.binding = 0;  
          attribute.format = RenderCore_PlatformVk::ResolveFormat(format);
          attribute.offset = overallVertexSize;
          attribute.location = reflectedInput.location;
          vertexAttributes.push_back(attribute);

          overallVertexSize += formatInfo.mySizeBytes;
        }
        compiledDataVk.myVertexAttributeDesc.myOverallVertexSize = overallVertexSize;
      }
      else if (aDesc.myShaderStage == (uint)ShaderStage::COMPUTE)
      {
        // SPIR-V reflect doesn't provide a way to reflect group-thread count yet, so just parse the source-code as a workaround for now
        String hlslSource = FileReader::ReadTextFile(hlslSrcPathAbs.c_str());
        ASSERT(hlslSource.size() > 0);

        size_t mainFuncPos = hlslSource.find("void " + aDesc.myMainFunction);
        ASSERT(mainFuncPos != String::npos);

        const char* numthreadsSearchKey = "[numthreads(";
        size_t numThreadsPos = hlslSource.rfind(numthreadsSearchKey, mainFuncPos);
        ASSERT(numThreadsPos != String::npos);

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
    }

    return true;
  }
//---------------------------------------------------------------------------//
}

