#include "fancy_core_precompile.h"
#include "DxcShaderCompiler.h"
#include "ShaderCompiler.h"
#include "IO/PathService.h"
#include "IO/FileReader.h"

#include <dxc/inc/dxcapi.h>
#include <atomic>

#include "Rendering/RenderCore.h"
#include "Rendering/Vulkan/RenderCore_PlatformVk.h"

namespace Fancy 
{
  namespace Priv_DxcShaderCompiler
  {
    struct IncludeHandler : IDxcIncludeHandler
    {
      eastl::fixed_vector<eastl::wstring, 16> myIncludeSearchPaths;
      IDxcLibrary* myDxcLibrary;

      IncludeHandler(IDxcLibrary* aDxcLibrary, eastl::string* someIncludeSearchPaths, uint aNumPaths)
        : myDxcLibrary(aDxcLibrary)
      {
        for (uint i = 0u; i < aNumPaths; ++i)
        {
          const eastl::string& dir = someIncludeSearchPaths[i];
          ASSERT(!dir.empty());

          if (dir[dir.size() - 1] != '/' && dir[dir.size() - 1] != '\\')
            myIncludeSearchPaths.push_back(StringUtil::ToWideString(dir + '/'));
          else
            myIncludeSearchPaths.push_back(StringUtil::ToWideString(dir));
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

        for (uint i = 0u; i < (uint) myIncludeSearchPaths.size(); ++i)
        {
          eastl::wstring path = myIncludeSearchPaths[i] + pFilename;
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
        return (ULONG)myRefCount;
      }

      ULONG Release() override
      {
        int refCount = --myRefCount;
        if (myRefCount == 0)
        {
          delete this;
        }
        return (ULONG)refCount;
      }

      std::atomic<int> myRefCount = 1;
    };

    StaticString<32> GetVkTargetEnvArg()
    {
      uint majorVersion, minorVersion;
      RenderCore::GetPlatformVk()->GetVulkanVersion(majorVersion, minorVersion);
      return StaticString<32>("-fspv-target-env=vulkan%d.%d", majorVersion, minorVersion);
    }
//---------------------------------------------------------------------------//
  }
//---------------------------------------------------------------------------//
  DxcShaderCompiler::DxcShaderCompiler()
  {
    HRESULT hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&myDxcLibrary));
    ASSERT(hr == S_OK, "Failed to create DXC library");
    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&myDxcCompiler));
    ASSERT(hr == S_OK, "Failed to create DXC compiler");
    hr = DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(&myDxcReflector));
    ASSERT(hr == S_OK, "Failed to create DXC container reflection");
  }
//---------------------------------------------------------------------------//
  DxcShaderCompiler::~DxcShaderCompiler()
  {
  }
//---------------------------------------------------------------------------//
  bool DxcShaderCompiler::CompileToBytecode(const char* anHlslSrcPathAbs, const ShaderDesc& aDesc, const Config& aConfig, eastl::vector<uint8>& aCompiledBytecodeOut) const
  {
    Microsoft::WRL::ComPtr<IDxcBlob> bytecodeBlob;
    if (!CompileToBytecode(anHlslSrcPathAbs, aDesc, aConfig, bytecodeBlob))
      return false;

    aCompiledBytecodeOut.resize((size_t)bytecodeBlob->GetBufferSize());
    memcpy(aCompiledBytecodeOut.data(), bytecodeBlob->GetBufferPointer(), bytecodeBlob->GetBufferSize());
    return true;
  }
//---------------------------------------------------------------------------//
  bool DxcShaderCompiler::CompileToBytecode(const char* anHlslSrcPathAbs, const ShaderDesc& aDesc, const Config& aConfig, Microsoft::WRL::ComPtr<IDxcBlob>& aCompiledBytecodeOut) const
  {
    eastl::string shaderFile = FileReader::ReadTextFile(anHlslSrcPathAbs);
    if (shaderFile.empty())
      return false;

    IDxcBlobEncoding* sourceBlob;
    if (myDxcLibrary->CreateBlobWithEncodingFromPinned(shaderFile.c_str(), (uint)shaderFile.size(), CP_UTF8, &sourceBlob) != S_OK)
      return false;

    LPCWSTR args[32];
    uint numArgs = 0u;

    auto AddArgument = [&](LPCWSTR anArg)
    {
      ASSERT(numArgs < ARRAY_LENGTH(args));
      args[numArgs++] = anArg;
    };

    AddArgument(L"/Zpc");                   // Pack matrices in column-major order

    if (aConfig.myDebug)
    {
      AddArgument(L"/Zi");                    // Enable debug information
      AddArgument(L"/o0");                    // Optimization level 0
      AddArgument(L"/od");                    // Disable optimizations
      AddArgument(L"/Qembed_debug");          // Silence warning about embedding PDBs into the shader container
    }

    eastl::wstring vkTargetEnvArg;
    if (aConfig.mySpirv)
    {
      AddArgument(L"-spirv");                 // Generate SPIR-V code
      // AddArgument(L"-fspv-reflect");          // Emit additional SPIR-V instructions to aid reflection
      //AddArgument(L"-fspv-extension=KHR");    // Allow all KHR extensions (including raytracing)
      //AddArgument(L"-fspv-extension=SPV_EXT_descriptor_indexing");
      //AddArgument(L"-fspv-extension=SPV_EXT_shader_stencil_export");
      //AddArgument(L"-fspv-extension=SPV_EXT_shader_viewport_index_layer");
      AddArgument(L"-fvk-use-dx-layout");     // Use DirectX memory layout for Vulkan resources
      AddArgument(L"-fvk-use-dx-position-w"); // Reciprocate SV_Position.w after reading from stage input in PS to accommodate the difference between Vulkan and DirectX
      if (aDesc.myShaderStage == (uint)ShaderStage::SHADERSTAGE_VERTEX)
        AddArgument(L"-fvk-invert-y");

      vkTargetEnvArg = StringUtil::ToWideString(Priv_DxcShaderCompiler::GetVkTargetEnvArg().GetBuffer());
      AddArgument(vkTargetEnvArg.c_str());
    }

    eastl::fixed_vector<eastl::wstring, 32> defineNames;
    eastl::fixed_vector<DxcDefine, 32> defines;
    for (const eastl::string& define : aDesc.myDefines)
    {
      defineNames.push_back(StringUtil::ToWideString(define.c_str()));
      defines.push_back({ defineNames[defineNames.size() - 1].c_str(), nullptr });
    }

    defineNames.push_back(L"DXC_COMPILER");
    defines.push_back({ defineNames[defineNames.size() - 1].c_str(), nullptr });

    if (aConfig.mySpirv)
    {
      defineNames.push_back(L"VULKAN");
      defines.push_back({ defineNames[defineNames.size() - 1].c_str(), nullptr });
    }

    const char* stageDefine = ShaderCompiler::ShaderStageToDefineString(static_cast<ShaderStage>(aDesc.myShaderStage));
    defineNames.push_back(StringUtil::ToWideString(stageDefine));
    defines.push_back({ defineNames[defineNames.size() - 1].c_str(), nullptr });

    eastl::string includePaths[] =
    {
      Path::GetContainingFolder(anHlslSrcPathAbs),
      Path::GetRootDirectory(),
    };
    Microsoft::WRL::ComPtr<Priv_DxcShaderCompiler::IncludeHandler> includeHandler =
      new Priv_DxcShaderCompiler::IncludeHandler(myDxcLibrary.Get(), includePaths, ARRAY_LENGTH(includePaths));

    eastl::wstring sourceName = StringUtil::ToWideString(aDesc.myPath);
    eastl::wstring mainFunction = StringUtil::ToWideString(aDesc.myMainFunction);
    eastl::wstring hlslProfileString = StringUtil::ToWideString(ShaderCompiler::GetHLSLprofileString(static_cast<ShaderStage>(aDesc.myShaderStage)).c_str());

    IDxcOperationResult* compiledResult;
    HRESULT result = myDxcCompiler->Compile(
      sourceBlob,
      sourceName.c_str(),
      mainFunction.c_str(),
      hlslProfileString.c_str(),
      args,
      numArgs,
      defines.data(),
      (uint) defines.size(),
      includeHandler.Get(),
      &compiledResult);

    IDxcBlobEncoding* errorBlob = nullptr;
    compiledResult->GetErrorBuffer(&errorBlob);

    if (errorBlob != nullptr)
    {
      IDxcBlobEncoding* errorBlob8 = nullptr;
      myDxcLibrary->GetBlobAsUtf8(errorBlob, &errorBlob8);

      if (errorBlob8 != nullptr && errorBlob8->GetBufferPointer() != nullptr && static_cast<const char*>(errorBlob8->GetBufferPointer())[0] != '\0')
      {
        LOG_ERROR("Error compiling shader %s: %s", aDesc.myPath.c_str(), static_cast<const char*>(errorBlob8->GetBufferPointer()));
        return false;
      }
    }
    else if(result != S_OK)
    {
      LOG_ERROR("Unknown error compiling shader %s", aDesc.myPath.c_str());
      return false;
    }

    IDxcBlob* resultBlob;
    result = compiledResult->GetResult(&resultBlob);
    if (result != S_OK)
    {
      resultBlob->Release();
      compiledResult->Release();
      LOG_ERROR("Failed getting compiled binary result of shader %s", aDesc.myPath.c_str());
      return false;
    }

    aCompiledBytecodeOut = resultBlob;

    return true;
  }
}


