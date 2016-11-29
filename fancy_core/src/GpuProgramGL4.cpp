#include "GpuProgramGL4.h"
#include "Serializer.h"

#if defined (RENDERER_OPENGL4)
#include "GpuProgramCompilerGL4.h"

namespace Fancy { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//
  void ShaderStageInterfaceElement::Serialize(IO::Serializer* aSerializer)
  {
    aSerializer->Serialize(&name, "name");
    aSerializer->Serialize(&uLocation, "location");
    aSerializer->Serialize(&uArraySize, "arraySize");
    aSerializer->Serialize(&uArrayStride, "arrayStride");
    aSerializer->Serialize(&uOffset, "offset");
    aSerializer->Serialize(&uBlockIndex, "blockIndex");
    aSerializer->Serialize(&eTypeGL, "eTypeGL");
    aSerializer->Serialize(&uAtomicCountBufIndex, "atomicCountBufIndex");
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  void ShaderStageFragmentOutput::Serialize(IO::Serializer* aSerializer)
  {
    aSerializer->Serialize(&name, "name");
    aSerializer->Serialize(&uRtIndex, "rtIndex");
    aSerializer->Serialize(&uLocation, "location");
    aSerializer->Serialize(&eFormat, "dataFormat");
    aSerializer->Serialize(&uFormatComponentCount, "formatComponentCount");
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  GpuProgramCompilerOutputGL4::GpuProgramCompilerOutputGL4() : 
    name(""),
    uProgramHandleGL(GLUINT_HANDLE_INVALID),
    eShaderStage(ShaderStage::NONE)
  {

  }
//---------------------------------------------------------------------------//
  GpuProgramGL4::GpuProgramGL4() :
    m_eShaderStage(ShaderStage::NONE),
    m_uProgramHandleGL(GLUINT_HANDLE_INVALID)
  {

  }
//---------------------------------------------------------------------------//
  GpuProgramGL4::~GpuProgramGL4()
  {

  }
//---------------------------------------------------------------------------//
  void GpuProgramGL4::SetFromCompilerOutput( const GpuProgramCompilerOutputGL4& _desc )
  {
    destroy();
    m_Name = _desc.name;
    m_uProgramHandleGL = _desc.uProgramHandleGL;
    m_eShaderStage = _desc.eShaderStage;
    m_clVertexInputLayout = _desc.clVertexInputLayout;
    m_vInputInterfaces = _desc.vInputInterfaces;
    m_vOutputInterfaces = _desc.vOutputInterfaces;
    m_vFragmentOutputs = _desc.vFragmentOutputs;
    m_vReadTextureInfos = _desc.vReadTextureInfos;
    m_vReadBufferInfos = _desc.vReadBufferInfos;
    m_vWriteTextureInfos = _desc.vWriteTextureInfos;
    m_vWriteBufferInfos = _desc.vWriteBufferInfos;
    myConstantBufferElements = _desc.myConstantBufferElements;
    myShaderCode = _desc.myShaderCode;
    myShaderFilename = _desc.myShaderFilename;
    myPermutation = _desc.myPermutation;
  }
//---------------------------------------------------------------------------//
  void GpuProgramGL4::Serialize(IO::Serializer* aSerializer)
  {
    if (aSerializer->getMode() == IO::ESerializationMode::LOAD)
      destroy();
    
    aSerializer->Serialize(&m_Name, "m_Name");
    aSerializer->Serialize(&m_eShaderStage, "ShaderStage");
    aSerializer->Serialize(&m_clVertexInputLayout, "VertexInputLayout");
    aSerializer->Serialize(&m_vInputInterfaces, "InputInterfaces");
    aSerializer->Serialize(&m_vOutputInterfaces, "OutputInterfaces");
    aSerializer->Serialize(&m_vFragmentOutputs, "FragmentOutpus");
    aSerializer->Serialize(&m_vReadTextureInfos, "ReadTextureInfos");
    aSerializer->Serialize(&m_vReadBufferInfos, "ReadBufferInfos");
    aSerializer->Serialize(&m_vWriteTextureInfos, "WriteTextureInfos");
    aSerializer->Serialize(&m_vWriteBufferInfos, "WriteBufferInfos");
    aSerializer->Serialize(&myConstantBufferElements, "ConstantBufferElements");
    aSerializer->Serialize(&myShaderCode, "ShaderCode");
    aSerializer->Serialize(&myShaderFilename, "ShaderFilename");
    aSerializer->Serialize(&myPermutation, "Permutation");

    if (aSerializer->getMode() == IO::ESerializationMode::LOAD)
      GpuProgramCompilerGL4::compileFromSource(myShaderCode, m_eShaderStage, m_uProgramHandleGL);
  }
//---------------------------------------------------------------------------//
  bool GpuProgramGL4::operator==(const GpuProgramDesc& anOtherDesc) const
  {
    const GpuProgramDesc& desc = GetDescription();
    return desc.myShaderPath == anOtherDesc.myShaderPath &&
      desc.myPermutation == anOtherDesc.myPermutation &&
      desc.myShaderStage == anOtherDesc.myShaderStage;
  }
//---------------------------------------------------------------------------//
  GpuProgramDesc GpuProgramGL4::GetDescription() const
  {
    GpuProgramDesc desc;
    desc.myShaderPath = myShaderFilename;
    desc.myPermutation = myPermutation;
    desc.myShaderStage = static_cast<uint32>(m_eShaderStage);
    return desc;
  }
//---------------------------------------------------------------------------//
  void GpuProgramGL4::SetFromDescription(const GpuProgramDesc& aDesc)
  {
    GpuProgramCompilerOutputGL4 compilerOutput;
    const bool success = GpuProgramCompilerGL4::Compile(aDesc, compilerOutput);

    if (success)
    {
      destroy();
      SetFromCompilerOutput(compilerOutput);
    }
  }
//---------------------------------------------------------------------------//
  void GpuProgramGL4::destroy()
  {
    if (m_uProgramHandleGL != GLUINT_HANDLE_INVALID) {
      glDeleteProgram(m_uProgramHandleGL);
      m_uProgramHandleGL = GLUINT_HANDLE_INVALID;
    }

    m_eShaderStage = ShaderStage::NONE;
  }
//---------------------------------------------------------------------------//
} } }  // end of namespace Fancy::Rendering::GL4

#endif
