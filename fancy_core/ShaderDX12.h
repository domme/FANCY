#pragma once

#include "FancyCoreDefines.h"
#include "DX12Prerequisites.h"
#include "Shader.h"
#include "RenderEnums.h"
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

    static VertexSemantics GetVertexSemanticFromShaderString(const char* aShaderString);
    static const char* GetShaderStringFromVertexSemantic(VertexSemantics aSemantic);
        
    const D3D12_INPUT_ELEMENT_DESC* GetNativeInputElements() const { ASSERT(!myNativeInputElements.empty()); return &myNativeInputElements[0u]; }
    uint GetNumNativeInputElements() const { return static_cast<uint>(myNativeInputElements.size()); }

    const DynamicArray<uint8>& GetBytecode() const { return myBytecode; }
    const D3D12_SHADER_BYTECODE& getNativeByteCode() const { return myNativeByteCode; }

    void SetFromCompilerOutput(const ShaderCompilerResult& aCompilerOutput) override;
    uint64 GetNativeBytecodeHash() const override;

    ID3D12RootSignature* GetRootSignature() const { return myRootSignature.Get(); }
    const RootSignatureLayoutDX12& GetRootSignatureLayout() const { return myRootSignatureLayout; }
    const DynamicArray<ShaderResourceInfoDX12>& GetResourceInfos() const { return myResourceInfos; }

  private:
    static void CreateNativeInputLayout(const ShaderVertexInputLayout& anInputLayout, std::vector<D3D12_INPUT_ELEMENT_DESC>& someNativeInputElements);
        
    DynamicArray<D3D12_INPUT_ELEMENT_DESC> myNativeInputElements;
    DynamicArray<uint8> myBytecode;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> myRootSignature;
    DynamicArray<ShaderResourceInfoDX12> myResourceInfos;
    RootSignatureLayoutDX12 myRootSignatureLayout;
    
    D3D12_SHADER_BYTECODE myNativeByteCode = {};
  };
//---------------------------------------------------------------------------//
}

#endif