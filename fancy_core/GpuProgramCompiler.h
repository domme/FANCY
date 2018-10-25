#pragma once

#include "RendererPrerequisites.h"
#include "GpuProgramProperties.h"
#include "VertexInputLayout.h"
#include "GpuProgramDesc.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct GpuProgramDesc;
  class ShaderResourceInterface;
//---------------------------------------------------------------------------//
  struct GpuProgramCompilerOutput
  { 
    String myShaderCode;
    ShaderResourceInterface* myRootSignature;
    void* myNativeData;

    GpuProgramDesc myDesc;
    GpuProgramProperties myProperties;
  };
//---------------------------------------------------------------------------//
  class GpuProgramCompiler
  {
    public:
      virtual ~GpuProgramCompiler() = default;
      virtual bool Compile(const GpuProgramDesc& aDesc, GpuProgramCompilerOutput* aCompilerOutput) const = 0;
      virtual String ResolvePlatformShaderPath(const String& aPath) const = 0;
  };
//---------------------------------------------------------------------------//
}
