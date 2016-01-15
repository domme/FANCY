#ifndef INCLUDE_GPUPROGRAMPIPELINEGL4_H
#define INCLUDE_GPUPROGRAMPIPELINEGL4_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

#if defined (RENDERER_OPENGL4)

#include "Serializable.h"

namespace Fancy{ namespace IO{
class Serializer;
} }

namespace Fancy { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//
  class GpuProgramPipelineGL4
  {
  public:
    void serialize(IO::Serializer* aSerializer);
    static ObjectName getTypeName() { return _N(GpuProgramPipeline); }
    ObjectName getName() const { return ObjectName(myShaderHash); }

    GpuProgramPipelineGL4();
    ~GpuProgramPipelineGL4();
    bool operator==(const GpuProgramPipelineGL4& anOther) const;
    bool operator==(const GpuProgramPipelineDesc& anOtherDesc) const;
    
    GpuProgramPipelineDesc GetDescription() const;
        
    GLuint GeneratePipelineHandleGL();
    void RecomputeHashFromShaders();

    GpuProgram* myGpuPrograms[(uint32)ShaderStage::NUM];

    uint myShaderHash;  /// Used to quickly compare two pipelines
    GLuint myPipelineHandleGL;
  };
//---------------------------------------------------------------------------//
} } } // end of namespace Fancy::Rendering:GL4

#endif

#endif  // INCLUDE_GPUPROGRAMPIPELINEGL4_H