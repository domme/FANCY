#include "GpuProgramGL4.h"
#include "Serializer.h"
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
  GpuProgramDescriptionGL4::GpuProgramDescriptionGL4() : 
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
  void GpuProgramGL4::init( const GpuProgramDescriptionGL4& _desc )
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

    if (aSerializer->getMode() == IO::ESerializationMode::LOAD)
      GpuProgramCompilerGL4::compileFromSource(myShaderCode, m_eShaderStage, m_uProgramHandleGL);
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
