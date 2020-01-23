#pragma once

#include "FancyCoreDefines.h"
#include "DX12Prerequisites.h"
#include "Shader.h"
#include "RenderEnums.h"
#include "ShaderResourceInfoDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct ShaderCompiledDataDX12
  {
    Microsoft::WRL::ComPtr<ID3DBlob> myBytecodeBlob;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> myRootSignature;
    DynamicArray<ShaderResourceInfoDX12> myResourceInfos;
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

    const Microsoft::WRL::ComPtr<ID3DBlob>& getNativeData() const { return myNativeData; }
    const D3D12_SHADER_BYTECODE& getNativeByteCode() const { return myNativeByteCode; }

    void SetFromCompilerOutput(const ShaderCompilerResult& aCompilerOutput) override;
    uint64 GetNativeBytecodeHash() const override;

    ID3D12RootSignature* GetRootSignature() const { return myRootSignature.Get(); }

  private:
    static void CreateNativeInputLayout(const ShaderVertexInputLayout& anInputLayout, std::vector<D3D12_INPUT_ELEMENT_DESC>& someNativeInputElements);
        
    DynamicArray<D3D12_INPUT_ELEMENT_DESC> myNativeInputElements;
    Microsoft::WRL::ComPtr<ID3DBlob> myNativeData;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> myRootSignature;
    DynamicArray<ShaderResourceInfoDX12> myResourceInfos;
    
    D3D12_SHADER_BYTECODE myNativeByteCode = {};
  };
//---------------------------------------------------------------------------//
}
