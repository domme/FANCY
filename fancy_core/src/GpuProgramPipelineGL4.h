#ifndef INCLUDE_GPUPROGRAMPIPELINEGL4_H
#define INCLUDE_GPUPROGRAMPIPELINEGL4_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"


namespace Fancy { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
  class GpuProgramPipelineGL4
  {
  public:
    GpuProgramPipelineGL4();
    ~GpuProgramPipelineGL4();

    bool hasStage(ShaderStage eStage) const {m_vGpuPrograms[(uint32)eStage] != nullptr;}
    

  protected:
    GpuProgramGL4* m_vGpuPrograms[(uint32)ShaderStage::NUM];


    GLuint m_uPipelineHandleGL;

  };
//---------------------------------------------------------------------------//
} } } // end of namespace Fancy::Rendering:GL4

#endif  // INCLUDE_GPUPROGRAMPIPELINEGL4_H