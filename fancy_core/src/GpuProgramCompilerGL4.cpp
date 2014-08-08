#include "GpuProgramCompilerGL4.h"
#include "GpuProgram.h"
#include "AdapterGL4.h"

namespace Fancy { namespace Core { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//
  GpuProgramCompilerGL4::GpuProgramCompilerGL4()
  {

  }
//---------------------------------------------------------------------------//
  GpuProgramCompilerGL4::~GpuProgramCompilerGL4()
  {

  }
//---------------------------------------------------------------------------//
  bool GpuProgramCompilerGL4::compileFromSource( const String& szSource, 
    const ShaderStage& eShaderStage, const ObjectName& name, GpuProgramGL4& rGpuProgram )
  {
    ASSERT_M(!szSource.empty(), "Invalid shader source");

    GLenum eShaderStageGL = Adapter::toGLType(eShaderStage);
    
    const char* szShaderSource_cstr = szSource.c_str();
    GLuint uProgramHandle = glCreateShaderProgramv(eShaderStageGL, 1, &szShaderSource_cstr);

    int iLogLengthChars = 0;
    glGetProgramiv(uProgramHandle, GL_INFO_LOG_LENGTH, &iLogLengthChars);
    ASSERT(iLogLengthChars < kMaxNumLogChars);

    if (iLogLengthChars > 0)
    {
      glGetProgramInfoLog(uProgramHandle, kMaxNumLogChars, &iLogLengthChars, m_LogBuffer);
      log_Info(m_LogBuffer);
    }
    // clear the log again
    memset(m_LogBuffer, 0x0, sizeof(m_LogBuffer));

    int iProgramLinkStatus = GL_FALSE;
    glGetProgramiv(uProgramHandle, GL_LINK_STATUS, &iProgramLinkStatus);

    bool success = iProgramLinkStatus;
    if (success)
    {
      rGpuProgram.m_eShaderStage = eShaderStage;
      rGpuProgram.m_uProgramHandleGL = uProgramHandle;
      rGpuProgram.m_Name = name;

      success = reflectProgram(rGpuProgram);
    }

    if (!success) 
    {
      log_Error(String("GpuProgram ") + name + " failed to compile" );
      glDeleteProgram(uProgramHandle);
    }
    
    return success;
  }
//---------------------------------------------------------------------------//
  bool GpuProgramCompilerGL4::reflectProgram( GpuProgramGL4& rGpuProgram )
  {
    GLuint uProgramHandle = rGpuProgram.m_uProgramHandleGL;
    ASSERT(uProgramHandle != GLUINT_HANDLE_INVALID);

    GLint iNumInputResources = 0;
    glGetProgramInterfaceiv(uProgramHandle, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &iNumInputResources);

    GLint iNumOutputResources = 0;
    glGetProgramInterfaceiv(uProgramHandle, GL_PROGRAM_OUTPUT, GL_ACTIVE_RESOURCES, &iNumOutputResources);

    GLenum eInterface = GL_PROGRAM_INPUT;
    for (uint32 i = 0u; i < iNumInputResources; ++i)
    {
      GLchar _name[128];
      glGetProgramResourceName(uProgramHandle, eInterface, i, _countof(_name), nullptr, _name);

      GLenum vProperties[] = {GL_TYPE, GL_LOCATION, GL_OFFSET};
      GLint vPropertyValues[_countof(vProperties)] = {0x0};

      glGetProgramResourceiv(uProgramHandle, eInterface, i,
        _countof(vProperties), vProperties, _countof(vPropertyValues), 
        nullptr, vPropertyValues );

      GLint _type = 

    }
    
  }
//---------------------------------------------------------------------------//
} } } } // end of namespace Fancy::Core::Rendering:GL4