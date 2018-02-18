#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "GpuProgramFeatures.h"
#include "GpuProgramDesc.h"
#include "VertexInputLayout.h"
#include "GpuProgramResource.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct GpuProgramCompilerOutput;
  class ShaderResourceInterface;
//---------------------------------------------------------------------------//
  class GpuProgram
  {
  public:

    GpuProgram();
    virtual ~GpuProgram() = default;

    GpuProgramDesc GetDescription() const;
    bool SetFromDescription(const GpuProgramDesc& aDesc, const GpuProgramCompiler* aCompiler);
    virtual void SetFromCompilerOutput(const GpuProgramCompilerOutput& aCompilerOutput);
    uint64 GetHash() const { return GetDescription().GetHash(); }
    virtual uint64 GetNativeBytecodeHash() const = 0;
    
    String mySourcePath;
    ShaderStage myStage;
    GpuProgramPermutation myPermutation;

    DynamicArray<GpuProgramResourceInfo> myReadTextureInfos;
    DynamicArray<GpuProgramResourceInfo> myReadBufferInfos;
    DynamicArray<GpuProgramResourceInfo> myWriteTextureInfos;
    DynamicArray<GpuProgramResourceInfo> myWriteBufferInfos;
    DynamicArray<ConstantBufferElement> myConstantBufferElements;

    ShaderVertexInputLayout myInputLayout;
    ShaderResourceInterface* myResourceInterface;
  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering
