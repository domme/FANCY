#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "MathUtil.h"
#include "Serializer.h"
#include "ShaderResourceInterface.h"

#include "GpuProgramPipeline.h"
#include "GpuProgram.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  GpuProgramPipeline::GpuProgramPipeline()
    : myShaderByteCodeHash(0u)
    , myResourceInterface(nullptr)
  {
    memset(myGpuPrograms, 0u, sizeof(myGpuPrograms));
  }
//---------------------------------------------------------------------------//
  GpuProgramPipeline::~GpuProgramPipeline()
  {

  }
//---------------------------------------------------------------------------//
  GpuProgramPipelineDesc GpuProgramPipeline::GetDescription() const
  {
    GpuProgramPipelineDesc desc;

    for (uint32 i = 0u; i < (uint32)ShaderStage::NUM; ++i)
    {
      const GpuProgram* pProgram = myGpuPrograms[i].get();
      if (pProgram)
        desc.myGpuPrograms[i] = pProgram->GetDescription();
    }

    return desc;
  }
//---------------------------------------------------------------------------//
  void GpuProgramPipeline::SetFromShaders(const std::array<SharedPtr<GpuProgram>, (uint32)ShaderStage::NUM>& someShaders)
  {
    for (uint32 i = 0u; i < (uint32)ShaderStage::NUM; ++i)
      myGpuPrograms[i] = someShaders[i];

    UpdateResourceInterface();
    UpdateShaderByteCodeHash();
  }
//---------------------------------------------------------------------------//
  void GpuProgramPipeline::UpdateResourceInterface()
  {
    myResourceInterface = nullptr;
    if (GpuProgram* vertexShader = myGpuPrograms[(uint32)ShaderStage::VERTEX].get())
    {
      myResourceInterface = vertexShader->GetResourceInterface();
    }
  }
//---------------------------------------------------------------------------//
  void GpuProgramPipeline::UpdateShaderByteCodeHash()
  {
    myShaderByteCodeHash = 0u;
    for (uint i = 0u; i < (uint)ShaderStage::NUM; ++i)
    {
      GpuProgram* shader = myGpuPrograms[i].get();
      MathUtil::hash_combine(myShaderByteCodeHash, reinterpret_cast<uint64>(shader));
      if (shader != nullptr)
        MathUtil::hash_combine(myShaderByteCodeHash, shader->GetNativeBytecodeHash());
    }
  }
//---------------------------------------------------------------------------//
} }