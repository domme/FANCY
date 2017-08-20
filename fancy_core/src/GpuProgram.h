#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "Serializable.h"
#include "GpuProgramFeatures.h"
#include "GpuProgramDesc.h"
#include "VertexInputLayout.h"
#include "GpuProgramResource.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  class ObjectFactory;
//---------------------------------------------------------------------------//
} }

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct GpuProgramCompilerOutput;
  class ShaderResourceInterface;
//---------------------------------------------------------------------------//
  class DLLEXPORT GpuProgram
  {
    friend class IO::ObjectFactory;

  public:
    SERIALIZABLE_RESOURCE(GpuProgram)

    GpuProgram();
    virtual ~GpuProgram() = default;

    GpuProgramDesc GetDescription() const;
    bool SetFromDescription(const GpuProgramDesc& aDesc, const GpuProgramCompiler* aCompiler);
    virtual void SetFromCompilerOutput(const GpuProgramCompilerOutput& aCompilerOutput);
    uint64 GetHash() const { return GetDescription().GetHash(); }
    virtual uint64 GetNativeBytecodeHash() const = 0;

    ShaderStage getShaderStage() const { return myStage; }
    const ShaderResourceInterface* GetResourceInterface() const { return myResourceInterface; }
    const GpuResourceInfoList& getReadTextureInfoList() const { return myReadTextureInfos; }
    const GpuResourceInfoList& getReadBufferInfoList() const { return myReadBufferInfos; }
    const GpuResourceInfoList& getWriteTextureInfoList() const { return myWriteTextureInfos; }
    const GpuResourceInfoList& getWriteBufferInfoList() const { return myWriteBufferInfos; }
    const ShaderVertexInputLayout* getVertexInputLayout() const { return &myInputLayout; }

  protected:
    String mySourcePath;
    ShaderStage myStage;
    GpuProgramPermutation myPermutation;

    GpuResourceInfoList myReadTextureInfos;
    GpuResourceInfoList myReadBufferInfos;
    GpuResourceInfoList myWriteTextureInfos;
    GpuResourceInfoList myWriteBufferInfos;
    ConstantBufferElementList myConstantBufferElements;

    ShaderVertexInputLayout myInputLayout;
    ShaderResourceInterface* myResourceInterface;
  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering
