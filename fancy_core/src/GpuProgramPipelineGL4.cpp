#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "Serializer.h"
#include "MathUtil.h"
#include "GpuProgram.h"
#include "GpuProgramCompilerGL4.h"

#if defined (RENDERER_OPENGL4)

#include "GpuProgramPipelineGL4.h"

namespace Fancy { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//
  GLuint locGetGLShaderStageBit(ShaderStage eShaderStage)
  {
    switch (eShaderStage)
    {
      case Fancy::Rendering::ShaderStage::VERTEX: return GL_VERTEX_SHADER_BIT;
      case Fancy::Rendering::ShaderStage::FRAGMENT: return GL_FRAGMENT_SHADER_BIT;
      case Fancy::Rendering::ShaderStage::GEOMETRY: return GL_GEOMETRY_SHADER_BIT;
      case Fancy::Rendering::ShaderStage::TESS_HULL: return GL_TESS_CONTROL_SHADER_BIT;
      case Fancy::Rendering::ShaderStage::TESS_DOMAIN: return GL_TESS_EVALUATION_SHADER_BIT;
      case Fancy::Rendering::ShaderStage::COMPUTE: return GL_COMPUTE_SHADER_BIT;
      default: ASSERT(false); return GL_VERTEX_SHADER_BIT;
    }
  }
//---------------------------------------------------------------------------//
  GpuProgramPipelineGL4::GpuProgramPipelineGL4() :
    myShaderHash(0u),
    myPipelineHandleGL(GLUINT_HANDLE_INVALID)
  {
    memset(myGpuPrograms, 0u, sizeof(myGpuPrograms));
  }
//---------------------------------------------------------------------------//
  GpuProgramPipelineGL4::~GpuProgramPipelineGL4()
  {
    Destroy();
  }
//---------------------------------------------------------------------------//
  void GpuProgramPipelineGL4::Destroy()
  {
    if (myPipelineHandleGL != GLUINT_HANDLE_INVALID)
      glDeleteProgramPipelines(1, &myPipelineHandleGL);

    myPipelineHandleGL = GLUINT_HANDLE_INVALID;
  }
//---------------------------------------------------------------------------//
  bool GpuProgramPipelineGL4::operator==(const GpuProgramPipelineGL4& anOther) const
  {
    return myShaderHash == anOther.myShaderHash;
  }
//---------------------------------------------------------------------------//
  GpuProgramPipelineDesc GpuProgramPipelineGL4::GetDescription() const
  {
    GpuProgramPipelineDesc desc;
    
    for (uint32 i = 0u; i < (uint32)ShaderStage::NUM; ++i)
    {
      const GpuProgram* pProgram = myGpuPrograms[i];
      if (pProgram)
        desc.myGpuPrograms[i] = pProgram->GetDescription();
    }

    return desc;
  }
//---------------------------------------------------------------------------//
  void GpuProgramPipelineGL4::SetFromDescription(const GpuProgramPipelineDesc& aDesc)
  {
    for (uint32 i = 0u; i < (uint32)ShaderStage::NUM; ++i)
    {
      myGpuPrograms[i] = GpuProgram::FindFromDesc(aDesc.myGpuPrograms[i]);
    }

    Destroy();
    myPipelineHandleGL = GeneratePipelineHandleGL();
    RecomputeHashFromShaders();
  }
//---------------------------------------------------------------------------//
  bool GpuProgramPipelineGL4::operator==(const GpuProgramPipelineDesc& anOtherDesc) const
  {
    return GetDescription() == anOtherDesc;
  }
//---------------------------------------------------------------------------//
  GLuint GpuProgramPipelineGL4::GeneratePipelineHandleGL()
  {
    if (myPipelineHandleGL != GLUINT_HANDLE_INVALID)
      glDeleteProgramPipelines(1, &myPipelineHandleGL);

    GLuint uPipeline;
    glGenProgramPipelines(1, &uPipeline);
    for (uint32 i = 0u; i < (uint32) ShaderStage::NUM; ++i)
    {
      const GpuProgram* pProgram = myGpuPrograms[i];
      if (pProgram)
      {
        GLuint shaderStageBit = locGetGLShaderStageBit(pProgram->getShaderStage());
        glUseProgramStages(uPipeline, shaderStageBit, pProgram->getProgramHandle());
      }
    }

    myPipelineHandleGL = uPipeline;
    return uPipeline;
  }
//---------------------------------------------------------------------------//
  void GpuProgramPipelineGL4::RecomputeHashFromShaders()
  {
    myShaderHash = 0u;
    for (uint i = 0u; i < (uint)ShaderStage::NUM; ++i)
    {
      MathUtil::hash_combine(myShaderHash, reinterpret_cast<uint>(myGpuPrograms[i]));
    }
  }
//---------------------------------------------------------------------------//
  void GpuProgramPipelineGL4::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serializeArray(myGpuPrograms, "myGpuPrograms");
  }
//---------------------------------------------------------------------------//
} } }

#endif  // RENDERER_OPENGL4