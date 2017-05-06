#pragma once

#include "DX12Prerequisites.h"
#include "GpuProgram.h"

namespace Fancy { namespace Rendering {
  class ShaderResourceInterface;
} }

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class GpuProgramDX12 : public GpuProgram
  {
    friend class GpuProgramCompilerDX12;
    friend class RenderOutputDX12;
    
  public:
    GpuProgramDX12();
    ~GpuProgramDX12() override;

    static VertexSemantics GetVertexSemanticFromShaderString(const char* aShaderString);
    static const char* GetShaderStringFromVertexSemantic(VertexSemantics aSemantic);
        
    const D3D12_INPUT_ELEMENT_DESC* GetNativeInputElements() const { ASSERT(!myNativeInputElements.empty()); return &myNativeInputElements[0u]; }
    uint32 GetNumNativeInputElements() const { return myNativeInputElements.size(); }

    const ComPtr<ID3DBlob>& getNativeData() const { return myNativeData; }
    const D3D12_SHADER_BYTECODE& getNativeByteCode() const { return myNativeByteCode; }

    void SetFromCompilerOutput(const GpuProgramCompilerOutput& aCompilerOutput) override;

    /// Shortcut for retrieving the DX12-rootSignature through the resourceInterface
    ID3D12RootSignature* GetRootSignature() const;

  private:
    static void CreateNativeInputLayout(const ShaderVertexInputLayout& anInputLayout, std::vector<D3D12_INPUT_ELEMENT_DESC>& someNativeInputElements);
        
    std::vector<D3D12_INPUT_ELEMENT_DESC> myNativeInputElements;

    ComPtr<ID3DBlob> myNativeData;
    D3D12_SHADER_BYTECODE myNativeByteCode;
  };
//---------------------------------------------------------------------------//
}}}