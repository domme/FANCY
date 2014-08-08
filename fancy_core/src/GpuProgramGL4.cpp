#include "GpuProgramGL4.h"

namespace Fancy { namespace Core { namespace Rendering { namespace GL4 {
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
  void GpuProgramGL4::destroy()
  {
    if (m_uProgramHandleGL != GLUINT_HANDLE_INVALID) {
      glDeleteProgram(m_uProgramHandleGL);
      m_uProgramHandleGL = GLUINT_HANDLE_INVALID;
    }

    m_eShaderStage = ShaderStage::NONE;
  }
//---------------------------------------------------------------------------//
} } } }  // end of namespace Fancy::Core::Rendering::GL4
