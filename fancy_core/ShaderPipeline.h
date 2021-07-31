#pragma once

#include "FancyCoreDefines.h"
#include "ShaderPipelineDesc.h"

#include "Ptr.h"
#include "Shader.h"
#include "Log.h"
#include "eastl/span.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class ShaderPipeline
  {
    public:
      ShaderPipeline();
      virtual ~ShaderPipeline();

      ShaderPipelineDesc GetDescription() const;
      void Create(const eastl::span<SharedPtr<Shader>, SHADERSTAGE_NUM>& someShaders);
      void Recreate();

      uint64 GetHash() const { return GetDescription().GetHash(); }
      uint64 GetShaderByteCodeHash() const { return myShaderByteCodeHash; }
      bool IsComputePipeline() const { return myShaders[SHADERSTAGE_COMPUTE] != nullptr; }
      const Shader* GetShader(ShaderStage aStage) const { return myShaders[(uint)aStage].get(); }
      const Shader* GetShader(uint aStage) const { ASSERT(aStage < SHADERSTAGE_NUM); return myShaders[aStage].get(); }

  protected:
      virtual void CreateFromShaders() = 0;
      void UpdateShaderByteCodeHash();

      SharedPtr<Shader> myShaders[SHADERSTAGE_NUM];
      uint64 myShaderByteCodeHash;  /// Can be used as "deep" comparison that is also affected when shaders are recompiled
  };
//---------------------------------------------------------------------------//
}