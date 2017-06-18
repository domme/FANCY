#pragma once

namespace Fancy { namespace Rendering {
  //---------------------------------------------------------------------------//
  class GpuProgramCompiler;
  class CommandContext;
  enum class CommandListType;
//---------------------------------------------------------------------------//  
  class RenderCore_Platform
  {
  public:
    virtual ~RenderCore_Platform() = default;

    virtual bool IsInitialized() = 0;

    virtual RenderOutput* CreateRenderOutput(void* aNativeInstanceHandle) = 0;
    virtual GpuProgramCompiler* CreateShaderCompiler() = 0;
    virtual GpuProgram* CreateGpuProgram() = 0;
    virtual Texture* CreateTexture() = 0;
    virtual GpuBuffer* CreateGpuBuffer() = 0;
    virtual CommandContext* CreateContext(CommandListType aType) = 0;
    virtual void InitBufferData(GpuBuffer* aBuffer, void* aDataPtr, CommandContext* aContext) = 0;
    virtual void UpdateBufferData(GpuBuffer* aBuffer, void* aDataPtr, uint32 aByteOffset, uint32 aByteSize, CommandContext* aContext) = 0;
    virtual void InitTextureData(Texture* aTexture, const TextureUploadData* someUploadDatas, uint32 aNumUploadDatas, CommandContext* aContext) = 0;
  };
  //---------------------------------------------------------------------------//
} }
