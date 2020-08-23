#pragma once

#include <wrl.h>

struct IDxcLibrary;
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
      bool mySpirv = false;
      String myProfile;
    };

    bool CompileToBytecode(const char* anHlslSrcPathAbs, const ShaderDesc& aDesc, const Config& aConfig, eastl::vector<uint8>& aCompiledBytecodeOut) const;
    bool CompileToBytecode(const char* anHlslSrcPathAbs, const ShaderDesc& aDesc, const Config& aConfig, Microsoft::WRL::ComPtr<IDxcBlob>& aCompiledBytecodeOut) const;

    IDxcContainerReflection* GetDxcReflector() const { return myDxcReflector.Get(); }

  private:
    Microsoft::WRL::ComPtr<IDxcLibrary> myDxcLibrary;
    Microsoft::WRL::ComPtr<IDxcCompiler> myDxcCompiler;
    Microsoft::WRL::ComPtr<IDxcContainerReflection> myDxcReflector;
  };
}


