#pragma once

#include <wrl.h>
#include "EASTL/string.h"

struct IDxcUtils;
struct IDxcCompiler;
struct ID3D10Blob;
struct IDxcContainerReflection;
struct IDxcBlob;

namespace Fancy
{
  struct ShaderDesc;

  class DxcShaderCompiler
  {
  public:
    DxcShaderCompiler();
    ~DxcShaderCompiler();

    struct Config
    {
      bool myDebug = true;
      eastl::string myProfile;
    };

    struct IncludeInfo
    {
      eastl::fixed_vector<eastl::string, 16> myIncludedFiles;
    };

    bool CompileToBytecode(const char* anHlslSrcPathAbs, const ShaderDesc& aDesc, const Config& aConfig, IncludeInfo& anIncludeInfo, eastl::vector<uint8>& aCompiledBytecodeOut) const;
    bool CompileToBytecode(const char* anHlslSrcPathAbs, const ShaderDesc& aDesc, const Config& aConfig, IncludeInfo& anIncludeInfo, Microsoft::WRL::ComPtr<IDxcBlob>& aCompiledBytecodeOut) const;

    IDxcContainerReflection* GetDxcReflector() const { return myDxcReflector.Get(); }

  private:
    Microsoft::WRL::ComPtr<IDxcUtils> myDxcUtils;
    Microsoft::WRL::ComPtr<IDxcCompiler> myDxcCompiler;
    Microsoft::WRL::ComPtr<IDxcContainerReflection> myDxcReflector;
  };
}


