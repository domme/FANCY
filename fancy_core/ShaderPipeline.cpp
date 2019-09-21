#include "fancy_core_precompile.h"
#include "ShaderPipeline.h"
#include "Shader.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  ShaderPipeline::ShaderPipeline()
    : myShaderByteCodeHash(0u)
  {
    memset(myShaders, 0u, sizeof(myShaders));
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
      const Shader* pProgram = myShaders[i].get();
      if (pProgram)
        desc.myShader[i] = pProgram->GetDescription();
    }

    return desc;
  }
//---------------------------------------------------------------------------//
  void ShaderPipeline::SetFromShaders(const FixedArray<SharedPtr<Shader>, (uint)ShaderStage::NUM>& someShaders)
  {
    for (uint i = 0u; i < (uint)ShaderStage::NUM; ++i)
      myShaders[i] = someShaders[i];

    UpdateResourceInterface();
    UpdateShaderByteCodeHash();
  }  
//---------------------------------------------------------------------------//
  void ShaderPipeline::UpdateShaderByteCodeHash()
  {
    myShaderByteCodeHash = 0u;
    for (uint i = 0u; i < (uint)ShaderStage::NUM; ++i)
    {
      Shader* shader = myShaders[i].get();
      MathUtil::hash_combine(myShaderByteCodeHash, reinterpret_cast<uint64>(shader));
      if (shader != nullptr)
        MathUtil::hash_combine(myShaderByteCodeHash, shader->GetNativeBytecodeHash());
    }
  }
//---------------------------------------------------------------------------//
}