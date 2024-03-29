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

    for (uint i = 0u; i < (uint)ShaderStage::SHADERSTAGE_NUM; ++i)
    {
      const Shader* pProgram = myShaders[i].get();
      if (pProgram)
        desc.myShader[i] = pProgram->GetDescription();
    }

    return desc;
  }
//---------------------------------------------------------------------------//
  void ShaderPipeline::Create(const eastl::span<SharedPtr<Shader>, (uint)ShaderStage::SHADERSTAGE_NUM>& someShaders)
  {
    for (uint i = 0u; i < (uint)ShaderStage::SHADERSTAGE_NUM; ++i)
      myShaders[i] = someShaders[i];

    Recreate();
  }
//---------------------------------------------------------------------------//
  void ShaderPipeline::Recreate()
  {
    CreateFromShaders();
    UpdateShaderByteCodeHash();
  }
//---------------------------------------------------------------------------//
  void ShaderPipeline::UpdateShaderByteCodeHash()
  {
    myShaderByteCodeHash = 0u;
    for (uint i = 0u; i < (uint)ShaderStage::SHADERSTAGE_NUM; ++i)
    {
      Shader* shader = myShaders[i].get();
      MathUtil::hash_combine(myShaderByteCodeHash, reinterpret_cast<uint64>(shader));
      if (shader != nullptr)
        MathUtil::hash_combine(myShaderByteCodeHash, shader->GetNativeBytecodeHash());
    }
  }
//---------------------------------------------------------------------------//
}