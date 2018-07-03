#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "RenderPlatformCaps.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class RenderOutput;
  class GpuProgramCompiler;
  class CommandContext;
  class CommandQueue;
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
    virtual GpuBuffer* CreateBuffer() = 0;
    virtual CommandContext* CreateContext(CommandListType aType) = 0;
    virtual DataFormat ResolveFormat(DataFormat aFormat) = 0;
    virtual CommandQueue* GetCommandQueue(CommandListType aType) = 0;
    virtual GpuResourceViewData* CreateTextureViewData(Texture* aTexture, const TextureViewProperties& someProperties) = 0;

  protected:
    RenderPlatformCaps myCaps;
  };
//---------------------------------------------------------------------------//
}
