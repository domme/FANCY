#pragma once

#include "GpuProgramProperties.h"
#include "GpuProgramDesc.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct GpuProgramDesc;
  class ShaderResourceInterface;
//---------------------------------------------------------------------------//
  struct GpuProgramCompilerOutput
  { 
    String myShaderCode;
    ShaderResourceInterface* myRootSignature = nullptr;
    void* myNativeData = nullptr;

    GpuProgramDesc myDesc;
    GpuProgramProperties myProperties;
  };
//---------------------------------------------------------------------------//
  class GpuProgramCompiler
  {
    public:
      virtual ~GpuProgramCompiler() = default;
      virtual String ResolvePlatformShaderPath(const String& aPath) const = 0;

      bool Compile(const GpuProgramDesc& aDesc, GpuProgramCompilerOutput* aCompilerOutput);

  protected:
    virtual bool Compile_Internal(const GpuProgramDesc& aDesc, const char** someDefines, uint aNumDefines, GpuProgramCompilerOutput* aCompilerOutput) const = 0;

  };
//---------------------------------------------------------------------------//
}
