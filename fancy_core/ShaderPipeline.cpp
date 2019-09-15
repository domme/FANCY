#include "fancy_core_precompile.h"
#include "ShaderPipeline.h"
#include "Shader.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  ShaderPipeline::ShaderPipeline()
    : myShaderByteCodeHash(0u)
  {
    memset(myGpuPrograms, 0u, sizeof(myGpuPrograms));
  }
//---------------------------------------------------------------------------//
  ShaderPipeline::~ShaderPipeline()
  {

  }
//---------------------------------------------------------------------------//
  ShaderPipelineDesc ShaderPipeline::GetDescription() const
  {
    ShaderPipelineDesc desc;

    for (uint i = 0u; i < (uint)ShaderStage::NUM; ++i)
    {
      const Shader* pProgram = myGpuPrograms[i].get();
      if (pProgram)
        desc.myGpuPrograms[i] = pProgram->GetDescription();
    }

    return desc;
  }
//---------------------------------------------------------------------------//
  void ShaderPipeline::SetFromShaders(const FixedArray<SharedPtr<Shader>, (uint)ShaderStage::NUM>& someShaders)
  {
    for (uint i = 0u; i < (uint)ShaderStage::NUM; ++i)
      myGpuPrograms[i] = someShaders[i];

    UpdateResourceInterface();
    UpdateShaderByteCodeHash();
  }  
//---------------------------------------------------------------------------//
  void ShaderPipeline::UpdateShaderByteCodeHash()
  {
    myShaderByteCodeHash = 0u;
    for (uint i = 0u; i < (uint)ShaderStage::NUM; ++i)
    {
      Shader* shader = myGpuPrograms[i].get();
      MathUtil::hash_combine(myShaderByteCodeHash, reinterpret_cast<uint64>(shader));
      if (shader != nullptr)
        MathUtil::hash_combine(myShaderByteCodeHash, shader->GetNativeBytecodeHash());
    }
  }
//---------------------------------------------------------------------------//
}