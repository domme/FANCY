#include "StdAfx.h"
#include "GpuProgramCompilerDX12.h"
#include "AdapterDX12.h"

#if defined (RENDERER_DX12)

#include "GpuProgramDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  void GpuProgramDX12::CreateNativeInputLayout(const ShaderVertexInputLayout& anInputLayout,
    std::vector<D3D12_INPUT_ELEMENT_DESC>& someNativeInputElements) const
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

      nativeElem.Format = Adapter::toNativeType(elem.myFormat);

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
    : myResourceInterface(nullptr)
  {
  }
//---------------------------------------------------------------------------//
  GpuProgramDX12::~GpuProgramDX12()
  {
  }
//---------------------------------------------------------------------------//
  void GpuProgramDX12::Destroy()
  {
  }
//---------------------------------------------------------------------------//
  bool GpuProgramDX12::operator==(const GpuProgramDX12& anOther) const
  {
    return GetDescription() == anOther.GetDescription();
  }
//---------------------------------------------------------------------------//
  bool GpuProgramDX12::operator==(const GpuProgramDesc& aDesc) const
  {
    return GetDescription() == aDesc;
  }
//---------------------------------------------------------------------------//
  void GpuProgramDX12::serialize(IO::Serializer* aSerializer)
  {
  }
//---------------------------------------------------------------------------//
  GpuProgramDesc GpuProgramDX12::GetDescription() const
  {
    GpuProgramDesc desc;
    desc.myShaderPath = mySourcePath;
    desc.myShaderStage = static_cast<uint32>(myStage);
    desc.myPermutation = myPermutation;
    return desc;
  }
//---------------------------------------------------------------------------//
  bool GpuProgramDX12::SetFromDescription(const GpuProgramDesc& aDesc)
  {
    GpuProgramCompilerOutputDX12 output;
    const bool success = GpuProgramCompilerDX12::Compile(aDesc, &output);

    if (success)
    {
      mySourcePath = aDesc.myShaderPath;
      myPermutation = aDesc.myPermutation;
      myStage = static_cast<ShaderStage>(aDesc.myShaderStage);

      Destroy();
      SetFromCompilerOutput(output);
    }

    return success;
  }
//---------------------------------------------------------------------------//
  void GpuProgramDX12::SetFromCompilerOutput(const GpuProgramCompilerOutputDX12& aCompilerOutput)
  {
    myNativeData = aCompilerOutput.myNativeData;
    myNativeByteCode.pShaderBytecode = myNativeData->GetBufferPointer();
    myNativeByteCode.BytecodeLength = myNativeData->GetBufferSize();

    myReadTextureInfos = aCompilerOutput.vReadTextureInfos;
    myReadBufferInfos = aCompilerOutput.vReadBufferInfos;
    myWriteTextureInfos = aCompilerOutput.vWriteTextureInfos;
    myWriteBufferInfos = aCompilerOutput.vWriteBufferInfos;
    myConstantBufferElements = aCompilerOutput.myConstantBufferElements;

    myStage = aCompilerOutput.eShaderStage;
    mySourcePath = aCompilerOutput.myShaderFilename;
    myPermutation = aCompilerOutput.myPermutation;
    myInputLayout = aCompilerOutput.clVertexInputLayout;
    CreateNativeInputLayout(myInputLayout, myNativeInputElements);

    myResourceInterface = aCompilerOutput.myRootSignature;
  }
//---------------------------------------------------------------------------//
} } }

#endif
