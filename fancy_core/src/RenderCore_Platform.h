#pragma once

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class GpuProgramCompiler;
  class CommandContext;
//---------------------------------------------------------------------------//  
  class RenderCore_Platform
  {
  public:
    virtual ~RenderCore_Platform() = default;

    virtual bool IsInitialized() = 0;

    virtual GpuProgramCompiler* CreateShaderCompiler() = 0;
    virtual GpuProgram* CreateGpuProgram() = 0;
    virtual Texture* CreateTexture() = 0;
    virtual GpuBuffer* CreateGpuBuffer() = 0;
    virtual CommandContext* CreateContext(CommandListType aType) = 0;
  };
//---------------------------------------------------------------------------//
} }
