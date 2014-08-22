#ifndef INCLUDE_GPUPROGRAMCOMPILERGL4_H
#define INCLUDE_GPUPROGRAMCOMPILERGL4_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "FixedArray.h"
#include "ObjectName.h"

namespace Fancy { namespace Core { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//
  class GpuProgramCompilerGL4
  {
  public:
    GpuProgramCompilerGL4();
    ~GpuProgramCompilerGL4();

    bool compileFromSource(const String& szSource, const ShaderStage& eShaderStage, 
      const ObjectName& name, GpuProgramGL4& rGpuProgram); 

  protected:
    bool reflectProgram(GpuProgramGL4& rGpuProgram);
    void reflectVertexInputs(GLuint uProgram, VertexInputLayout& rVertexLayout) const;
    void reflectFragmentOutputs(GLuint uProgram, ShaderStageFragmentOutputList& vFragmentOutputs) const;
    void reflectStageInputs(GLuint uProgram, ShaderStageInterfaceList& rInterfaceList) const;
    void reflectStageOutputs(GLuint uProgram, ShaderStageInterfaceList& rInterfaceList) const;
    

    const uint32 kMaxNumLogChars = 10000u;
    char m_LogBuffer[kMaxNumLogChars];
  };
//---------------------------------------------------------------------------//
} } } } // end of namespace Fancy::Core::Rendering:GL4

#endif  // INCLUDE_GPUPROGRAMPIPELINEGL4_H