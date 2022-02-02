#pragma once

#include "Common/FancyCoreDefines.h"
#include "Rendering/Shader.h"

#include "DX12Prerequisites.h"

#include "eastl/vector.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
//---------------------------------------------------------------------------//
  struct ShaderCompiledDataDX12
  {
    eastl::vector<uint8> myBytecode;
  };
//---------------------------------------------------------------------------//
  class ShaderDX12 : public Shader
  {
    friend class ShaderCompilerDX12;
    
  public:
    ShaderDX12() = default;
    ~ShaderDX12() override = default;

    const eastl::vector<uint8>& GetBytecode() const { return myCompiledData.myBytecode; }
    const D3D12_SHADER_BYTECODE& getNativeByteCode() const { return myNativeByteCode; }

    void SetFromCompilerOutput(const ShaderCompilerResult& aCompilerOutput) override;
    uint64 GetNativeBytecodeHash() const override { return myNativeByteCodeHash; }

  private:
    ShaderCompiledDataDX12 myCompiledData;
    
    D3D12_SHADER_BYTECODE myNativeByteCode = {};
    uint64 myNativeByteCodeHash = 0ull;
  };
//---------------------------------------------------------------------------//
}

#endif