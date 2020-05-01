#pragma once

#include "FancyCoreDefines.h"
#include "ShaderPipelineDesc.h"

#include "FixedArray.h"
#include "Ptr.h"
#include "Shader.h"
#include "Log.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class ShaderPipeline
  {
    public:
      ShaderPipeline();
      virtual ~ShaderPipeline();

      ShaderPipelineDesc GetDescription() const;
      void Create(const FixedArray<SharedPtr<Shader>, (uint)ShaderStage::NUM>& someShaders);
      void Recreate();

      uint64 GetHash() const { return GetDescription().GetHash(); }
      uint64 GetShaderByteCodeHash() const { return myShaderByteCodeHash; }
      bool IsComputePipeline() const { return myShaders[(uint)ShaderStage::COMPUTE] != nullptr; }
      const Shader* GetShader(ShaderStage aStage) const { return myShaders[(uint)aStage].get(); }
      const Shader* GetShader(uint aStage) const { ASSERT(aStage < (uint)ShaderStage::NUM); return myShaders[aStage].get(); }

  protected:
      virtual void CreateFromShaders() = 0;
      void UpdateShaderByteCodeHash();

      SharedPtr<Shader> myShaders[(uint)ShaderStage::NUM];
      uint64 myShaderByteCodeHash;  /// Can be used as "deep" comparison that is also affected when shaders are recompiled
  };
//---------------------------------------------------------------------------//
}