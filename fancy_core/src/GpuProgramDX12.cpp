#include "StdAfx.h"
#include "GpuProgramCompiler.h"
#include "AdapterDX12.h"

#include "ShaderResourceInterface.h"
#include "GpuProgramCompiler.h"

#include "GpuProgramDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"
#include "ShaderResourceInterfaceDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  void GpuProgramDX12::CreateNativeInputLayout(const ShaderVertexInputLayout& anInputLayout,
    std::vector<D3D12_INPUT_ELEMENT_DESC>& someNativeInputElements)
  {
    someNativeInputElements.clear();

    const ShaderVertexInputElementList& inputElements = anInputLayout.getVertexElementList();

    uint32 offsetByte = 0u;
    for (uint32 i = 0u; i < inputElements.size(); ++i)
    {
      const ShaderVertexInputElement& elem = inputElements[i];
      
      D3D12_INPUT_ELEMENT_DESC nativeElem = { 0u };

      nativeElem.SemanticIndex = elem.mySemanticIndex;
      nativeElem.SemanticName = GetShaderStringFromVertexSemantic(elem.mySemantics);

      nativeElem.AlignedByteOffset = offsetByte;
      offsetByte += elem.mySizeBytes;

      nativeElem.Format = RenderCore_PlatformDX12::GetFormat(elem.myFormat);

      // TODO: Add support for instancing
      nativeElem.InputSlot = 0;
      nativeElem.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

      someNativeInputElements.push_back(nativeElem);
    }
  }
//---------------------------------------------------------------------------//
  VertexSemantics GpuProgramDX12::GetVertexSemanticFromShaderString(const char* aShaderString)
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
  const char* GpuProgramDX12::GetShaderStringFromVertexSemantic(VertexSemantics aSemantic)
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
  GpuProgramDX12::GpuProgramDX12()
  {
  }
//---------------------------------------------------------------------------//
  GpuProgramDX12::~GpuProgramDX12()
  {
  }
//---------------------------------------------------------------------------//
  void GpuProgramDX12::SetFromCompilerOutput(const GpuProgramCompilerOutput& aCompilerOutput)
  {
    GpuProgram::SetFromCompilerOutput(aCompilerOutput);

    myNativeData = static_cast<ID3DBlob*>(aCompilerOutput.myNativeData);
    myNativeByteCode.pShaderBytecode = myNativeData->GetBufferPointer();
    myNativeByteCode.BytecodeLength = myNativeData->GetBufferSize();

    CreateNativeInputLayout(myInputLayout, myNativeInputElements);
  }
//---------------------------------------------------------------------------//
  ID3D12RootSignature* GpuProgramDX12::GetRootSignature() const
  {
    return static_cast<ShaderResourceInterfaceDX12*>(myResourceInterface)->myRootSignature.Get();
  }
//---------------------------------------------------------------------------//
  uint64 GpuProgramDX12::GetNativeBytecodeHash() const
  {
    return reinterpret_cast<uint64>(getNativeData().Get());
  }
//---------------------------------------------------------------------------//
} } }
