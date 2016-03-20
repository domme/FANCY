#include "StdAfx.h"
#include "GpuProgramCompilerDX12.h"
#include "AdapterDX12.h"

#if defined (RENDERER_DX12)

#include "GpuProgramDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  static void locCreateNativeInputLayout(const ShaderVertexInputLayout& anInputLayout, 
    std::vector<D3D12_INPUT_ELEMENT_DESC>& someNativeInputElements)
  {
    someNativeInputElements.clear();

    const ShaderVertexInputElementList& inputElements = anInputLayout.getVertexElementList();

    uint32 offsetByte = 0u;
    for (uint32 i = 0u; i < inputElements.size(); ++i)
    {
      const ShaderVertexInputElement& elem = inputElements[i];
      
      D3D12_INPUT_ELEMENT_DESC nativeElem = { 0u };

      switch (elem.mySemantics)
      {
        case VertexSemantics::POSITION: 
          nativeElem.SemanticName = "POSITION";
          break;
        case VertexSemantics::NORMAL: 
          nativeElem.SemanticName = "NORMAL";
          break;
        case VertexSemantics::TANGENT: 
          nativeElem.SemanticName = "TANGENT";
          break;
        case VertexSemantics::BITANGENT: 
          nativeElem.SemanticName = "BITANGENT"; 
          break;
        case VertexSemantics::COLOR: 
          nativeElem.SemanticName = "COLOR";
          break;
        case VertexSemantics::TEXCOORD: 
          nativeElem.SemanticName = "TEXCOORD";
          break;
        default:
          ASSERT(false);
      }

      nativeElem.SemanticIndex = elem.mySemanticIndex;

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
  GpuProgramDX12::GpuProgramDX12()
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
    locCreateNativeInputLayout(myInputLayout, myNativeInputElements);

    myRootSignature = aCompilerOutput.myRootSignature;
  }
//---------------------------------------------------------------------------//
} } }

#endif
