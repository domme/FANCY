#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "RenderPlatformCaps.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class RenderOutput;
  class GpuProgramCompiler;
  class CommandContext;
  enum class CommandListType;
//---------------------------------------------------------------------------//  
  class RenderCore_Platform
  {
  public:
    virtual ~RenderCore_Platform() = default;

    virtual bool IsInitialized() = 0;
    virtual bool InitInternalResources() = 0;
    virtual void InitCaps() = 0;

    const RenderPlatformCaps& GetCaps() const { return myCaps; }
    virtual RenderOutput* CreateRenderOutput(void* aNativeInstanceHandle) = 0;
    virtual GpuProgramCompiler* CreateShaderCompiler() = 0;
    virtual GpuProgram* CreateGpuProgram() = 0;
    virtual Texture* CreateTexture() = 0;
    virtual GpuBuffer* CreateGpuBuffer() = 0;
    virtual CommandContext* CreateContext(CommandListType aType) = 0;
    virtual void InitBufferData(GpuBuffer* aBuffer, const void* aDataPtr, CommandContext* aContext) = 0;
    virtual void UpdateBufferData(GpuBuffer* aBuffer, void* aDataPtr, uint aByteOffset, uint aByteSize, CommandContext* aContext) = 0;
    virtual void InitTextureData(Texture* aTexture, const TextureUploadData* someUploadDatas, uint aNumUploadDatas, CommandContext* aContext) = 0;
    virtual DataFormat ResolveFormat(DataFormat aFormat) = 0;

  protected:
    RenderPlatformCaps myCaps;
  };
//---------------------------------------------------------------------------//
}
