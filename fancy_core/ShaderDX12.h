#pragma once

#include "FancyCoreDefines.h"
#include "DX12Prerequisites.h"
#include "Shader.h"
#include "ShaderResourceInfoDX12.h"
#include "RootSignatureDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
//---------------------------------------------------------------------------//
  struct ShaderCompiledDataDX12
  {
    DynamicArray<uint8> myBytecode;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> myRootSignature;
    DynamicArray<ShaderResourceInfoDX12> myResourceInfos;
    RootSignatureLayoutDX12 myRootSignatureLayout;
  };
//---------------------------------------------------------------------------//
  class ShaderDX12 : public Shader
  {
    friend class ShaderCompilerDX12;
    
  public:
    ShaderDX12() = default;
    ~ShaderDX12() override = default;

    const DynamicArray<uint8>& GetBytecode() const { return myBytecode; }
    const D3D12_SHADER_BYTECODE& getNativeByteCode() const { return myNativeByteCode; }

    void SetFromCompilerOutput(const ShaderCompilerResult& aCompilerOutput) override;
    uint64 GetNativeBytecodeHash() const override { return myNativeByteCodeHash; }

    ID3D12RootSignature* GetRootSignature() const { return myRootSignature.Get(); }
    const RootSignatureLayoutDX12& GetRootSignatureLayout() const { return myRootSignatureLayout; }
    const DynamicArray<ShaderResourceInfoDX12>& GetResourceInfos() const { return myResourceInfos; }

  private:
    DynamicArray<uint8> myBytecode;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> myRootSignature;
    DynamicArray<ShaderResourceInfoDX12> myResourceInfos;
    RootSignatureLayoutDX12 myRootSignatureLayout;
    
    D3D12_SHADER_BYTECODE myNativeByteCode = {};
    uint64 myNativeByteCodeHash = 0ull;
  };
//---------------------------------------------------------------------------//
}

#endif