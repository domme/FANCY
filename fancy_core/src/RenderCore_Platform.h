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

    virtual SharedPtr<GpuProgramCompiler> CreateShaderCompiler() = 0;
    virtual SharedPtr<GpuProgram> CreateGpuProgram() = 0;
    virtual SharedPtr<GpuProgramPipeline> CreateGpuProgramPipeline() = 0;
    virtual SharedPtr<Texture> CreateTexture() = 0;
    virtual GpuBuffer* CreateGpuBuffer() = 0;
    virtual CommandContext* CreateContext(CommandListType aType) = 0;
  };
//---------------------------------------------------------------------------//
} }
