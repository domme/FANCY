#pragma once

#include "RendererPrerequisites.h"
#include "GpuProgramResource.h"
#include "VertexInputLayout.h"
#include "GpuProgramDesc.h"

namespace Fancy { namespace Rendering {
  struct GpuProgramDesc;
  class ShaderResourceInterface;
} }

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct GpuProgramCompilerOutput
  {
    ShaderVertexInputLayout clVertexInputLayout;
    GpuResourceInfoList vReadTextureInfos;
    GpuResourceInfoList vReadBufferInfos;
    GpuResourceInfoList vWriteTextureInfos;
    GpuResourceInfoList vWriteBufferInfos;
    ConstantBufferElementList myConstantBufferElements;
    ShaderStage eShaderStage;
    String myShaderCode;
    String myShaderFilename;  /// Platform-independent shader filename (e.g. "MaterialForward")
    GpuProgramPermutation myPermutation;
    ShaderResourceInterface* myRootSignature;
    void* myNativeData;
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
} } // end of namespace Fancy::Rendering
