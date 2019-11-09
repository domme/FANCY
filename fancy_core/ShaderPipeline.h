#pragma once

#include "FancyCoreDefines.h"
#include "ShaderPipelineDesc.h"

#include "FixedArray.h"
#include "Ptr.h"
#include "Shader.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class ShaderPipeline
  {
    public:
      ShaderPipeline();
      virtual ~ShaderPipeline();

      ShaderPipelineDesc GetDescription() const;
      void SetFromShaders(const FixedArray<SharedPtr<Shader>, (uint)ShaderStage::NUM>& someShaders);

      uint64 GetHash() const { return GetDescription().GetHash(); }
      uint64 GetShaderByteCodeHash() const { return myShaderByteCodeHash; }

      virtual void UpdateResourceInterface() = 0;
      void UpdateShaderByteCodeHash();

      SharedPtr<Shader> myShaders[(uint)ShaderStage::NUM];
      uint64 myShaderByteCodeHash;  /// Can be used as "deep" comparison that is also affected when shaders are recompiled
  };
//---------------------------------------------------------------------------//
}