#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "GpuProgramPipelineDesc.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class ShaderResourceInterface;
//---------------------------------------------------------------------------//
  class GpuProgramPipeline
  {
    public:
      GpuProgramPipeline();
      virtual ~GpuProgramPipeline();

      GpuProgramPipelineDesc GetDescription() const;
      void SetFromShaders(const std::array<SharedPtr<GpuProgram>, (uint)ShaderStage::NUM>& someShaders);

      uint64 GetHash() const { return GetDescription().GetHash(); }
      uint64 GetShaderByteCodeHash() const { return myShaderByteCodeHash; }

      void UpdateResourceInterface();
      void UpdateShaderByteCodeHash();

      SharedPtr<GpuProgram> myGpuPrograms[(uint)ShaderStage::NUM];
      uint64 myShaderByteCodeHash;  /// Can be used as "deep" comparison that is also affected when shaders are recompiled
      const ShaderResourceInterface* myResourceInterface;
  };
//---------------------------------------------------------------------------//
}