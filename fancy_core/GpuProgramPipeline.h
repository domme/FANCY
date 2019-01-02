#pragma once

#include "FancyCoreDefines.h"
#include "GpuProgramPipelineDesc.h"

#include "FixedArray.h"
#include "Ptr.h"
#include "GpuProgram.h"

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
      void SetFromShaders(const FixedArray<SharedPtr<GpuProgram>, (uint)ShaderStage::NUM>& someShaders);

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