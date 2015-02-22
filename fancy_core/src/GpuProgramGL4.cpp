#include "GpuProgramGL4.h"

namespace Fancy { namespace Rendering { namespace GL4 {
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
