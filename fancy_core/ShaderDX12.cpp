#include "fancy_core_precompile.h"
#include "ShaderDX12.h"

#include "ShaderCompiler.h"
#include "RenderCore_PlatformDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  void ShaderDX12::CreateNativeInputLayout(const ShaderVertexInputLayout& anInputLayout,
    std::vector<D3D12_INPUT_ELEMENT_DESC>& someNativeInputElements)
  {
    someNativeInputElements.clear();

    const DynamicArray<ShaderVertexInputElement>& inputElements = anInputLayout.myVertexInputElements;

    uint offsetByte = 0u;
    for (uint i = 0u; i < inputElements.size(); ++i)
    {
      const ShaderVertexInputElement& elem = inputElements[i];
      
      D3D12_INPUT_ELEMENT_DESC nativeElem = { 0u };

      nativeElem.SemanticIndex = elem.mySemanticIndex;
      nativeElem.SemanticName = GetShaderStringFromVertexSemantic(elem.mySemantics);

      nativeElem.AlignedByteOffset = offsetByte;
      offsetByte += elem.mySizeBytes;

      nativeElem.Format = RenderCore_PlatformDX12::GetDXGIformat(elem.myFormat);

      // TODO: Add support for instancing
      nativeElem.InputSlot = 0;
      nativeElem.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

      someNativeInputElements.push_back(nativeElem);
    }
  }
//---------------------------------------------------------------------------//
  VertexSemantics ShaderDX12::GetVertexSemanticFromShaderString(const char* aShaderString)
  {
    if (strcmp(aShaderString, "POSITION") == 0u)
      return VertexSemantics::POSITION;
    else if (strcmp(aShaderString, "NORMAL") == 0u)
      return VertexSemantics::NORMAL;
    else if (strcmp(aShaderString, "TANGENT") == 0u)
      return VertexSemantics::TANGENT;
    else if (strcmp(aShaderString, "TEXCOORD") == 0u)
      return VertexSemantics::TEXCOORD;
    else if (strcmp(aShaderString, "COLOR") == 0u)
      return VertexSemantics::COLOR;
    else if (strcmp(aShaderString, "BITANGENT") == 0u
      || strcmp(aShaderString, "BINORMAL") == 0u)
      return VertexSemantics::BITANGENT;

    ASSERT(false, "Unknown vertex semantics");
    return VertexSemantics::NONE;
  }
//---------------------------------------------------------------------------//
  const char* ShaderDX12::GetShaderStringFromVertexSemantic(VertexSemantics aSemantic)
  {
    switch (aSemantic)
    {
    case VertexSemantics::POSITION: return "POSITION";
    case VertexSemantics::NORMAL: return "NORMAL";
    case VertexSemantics::TANGENT: return "TANGENT";
    case VertexSemantics::BITANGENT: return "BINORMAL";
    case VertexSemantics::COLOR: return "COLOR";
    case VertexSemantics::TEXCOORD: return "TEXCOORD";
    default:
      ASSERT(false); return "POSITION";
    }
  }
//---------------------------------------------------------------------------//
  void ShaderDX12::SetFromCompilerOutput(const ShaderCompilerResult& aCompilerOutput)
  {
    Shader::SetFromCompilerOutput(aCompilerOutput);

    const ShaderCompiledDataDX12& data = aCompilerOutput.myNativeData.To<ShaderCompiledDataDX12>();

    myNativeData = data.myBytecodeBlob;
    myRootSignature = data.myRootSignature;

    myNativeByteCode.pShaderBytecode = myNativeData->GetBufferPointer();
    myNativeByteCode.BytecodeLength = myNativeData->GetBufferSize();
    
    CreateNativeInputLayout(myProperties.myVertexInputLayout, myNativeInputElements);
  }
//---------------------------------------------------------------------------//
  uint64 ShaderDX12::GetNativeBytecodeHash() const
  {
    return reinterpret_cast<uint64>(getNativeData().Get());
  }
//---------------------------------------------------------------------------//
}
