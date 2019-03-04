#pragma once

#include "RendererPrerequisites.h"
#include "Ptr.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class RenderOutput;
  class GpuProgramCompiler;
  class CommandContext;
  class CommandQueue;
  enum class CommandListType;
  struct GpuResourceViewData;
  struct TextureViewProperties;
  struct WindowParameters;
  class TextureView;
  class GpuBufferView;
  struct GpuBufferViewProperties;
  class GpuProgram;
  class Texture;
  class GpuBuffer;
  class GpuQueryHeap;
//---------------------------------------------------------------------------//  
  class RenderCore_Platform
  {
  public:
    virtual ~RenderCore_Platform() = default;

    virtual bool IsInitialized() = 0;
    virtual bool InitInternalResources() = 0;
    virtual void InitCaps() = 0;
    virtual void Shutdown() = 0;

    const RenderPlatformCaps& GetCaps() const { return myCaps; }
    virtual RenderOutput* CreateRenderOutput(void* aNativeInstanceHandle, const WindowParameters& someWindowParams) = 0;
    virtual GpuProgramCompiler* CreateShaderCompiler() = 0;
    virtual GpuProgram* CreateGpuProgram() = 0;
    virtual Texture* CreateTexture() = 0;
    virtual GpuBuffer* CreateBuffer() = 0;
    virtual CommandContext* CreateContext(CommandListType aType) = 0;
    virtual CommandQueue* GetCommandQueue(CommandListType aType) = 0;
    virtual TextureView* CreateTextureView(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties, const char* aDebugName = nullptr) = 0;
    virtual GpuBufferView* CreateBufferView(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties, const char* aDebugName = nullptr) = 0;
    virtual GpuQueryHeap* CreateQueryHeap(GpuQueryType aType, uint aNumQueries) = 0;
    virtual uint GetQueryTypeDataSize(GpuQueryType aType) = 0;
    virtual float64 GetTimestampToSecondsFactor(CommandListType aCommandListType) = 0;
  protected:
    RenderPlatformCaps myCaps;
  };
//---------------------------------------------------------------------------//
}
