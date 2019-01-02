#include "fancy_core_precompile.h"
#include "GpuProgramPipeline.h"
#include "GpuProgram.h"

namespace Fancy {
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

    for (uint i = 0u; i < (uint)ShaderStage::NUM; ++i)
    {
      const GpuProgram* pProgram = myGpuPrograms[i].get();
      if (pProgram)
        desc.myGpuPrograms[i] = pProgram->GetDescription();
    }

    return desc;
  }
//---------------------------------------------------------------------------//
  void GpuProgramPipeline::SetFromShaders(const FixedArray<SharedPtr<GpuProgram>, (uint)ShaderStage::NUM>& someShaders)
  {
    for (uint i = 0u; i < (uint)ShaderStage::NUM; ++i)
      myGpuPrograms[i] = someShaders[i];

    UpdateResourceInterface();
    UpdateShaderByteCodeHash();
  }
//---------------------------------------------------------------------------//
  void GpuProgramPipeline::UpdateResourceInterface()
  {
    myResourceInterface = nullptr;
    if (GpuProgram* vertexShader = myGpuPrograms[(uint)ShaderStage::VERTEX].get())
    {
      myResourceInterface = vertexShader->myResourceInterface;
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
}