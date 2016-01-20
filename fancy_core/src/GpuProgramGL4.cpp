#include "GpuProgramGL4.h"
#include "Serializer.h"

#if defined (RENDERER_OPENGL4)
#include "GpuProgramCompilerGL4.h"

namespace Fancy { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//
  void ShaderStageInterfaceElement::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&name, "name");
    aSerializer->serialize(&uLocation, "location");
    aSerializer->serialize(&uArraySize, "arraySize");
    aSerializer->serialize(&uArrayStride, "arrayStride");
    aSerializer->serialize(&uOffset, "offset");
    aSerializer->serialize(&uBlockIndex, "blockIndex");
    aSerializer->serialize(&eTypeGL, "eTypeGL");
    aSerializer->serialize(&uAtomicCountBufIndex, "atomicCountBufIndex");
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  void ShaderStageFragmentOutput::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&name, "name");
    aSerializer->serialize(&uRtIndex, "rtIndex");
    aSerializer->serialize(&uLocation, "location");
    aSerializer->serialize(&eFormat, "dataFormat");
    aSerializer->serialize(&uFormatComponentCount, "formatComponentCount");
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
  void GpuProgramGL4::serialize(IO::Serializer* aSerializer)
  {
    if (aSerializer->getMode() == IO::ESerializationMode::LOAD)
      destroy();
    
    aSerializer->serialize(&m_Name, "m_Name");
    aSerializer->serialize(&m_eShaderStage, "ShaderStage");
    aSerializer->serialize(&m_clVertexInputLayout, "VertexInputLayout");
    aSerializer->serialize(&m_vInputInterfaces, "InputInterfaces");
    aSerializer->serialize(&m_vOutputInterfaces, "OutputInterfaces");
    aSerializer->serialize(&m_vFragmentOutputs, "FragmentOutpus");
    aSerializer->serialize(&m_vReadTextureInfos, "ReadTextureInfos");
    aSerializer->serialize(&m_vReadBufferInfos, "ReadBufferInfos");
    aSerializer->serialize(&m_vWriteTextureInfos, "WriteTextureInfos");
    aSerializer->serialize(&m_vWriteBufferInfos, "WriteBufferInfos");
    aSerializer->serialize(&myConstantBufferElements, "ConstantBufferElements");
    aSerializer->serialize(&myShaderCode, "ShaderCode");
    aSerializer->serialize(&myShaderFilename, "ShaderFilename");
    aSerializer->serialize(&myPermutation, "Permutation");

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
