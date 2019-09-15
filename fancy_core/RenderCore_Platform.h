#pragma once

#include "RendererPrerequisites.h"
#include "Ptr.h"
#include "GpuResourceStateTracking.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class RenderOutput;
  class ShaderCompiler;
  class CommandList;
  class CommandQueue;
  enum class CommandListType;
  struct GpuResourceViewData;
  struct TextureViewProperties;
  struct WindowParameters;
  class TextureView;
  class GpuBufferView;
  struct GpuBufferViewProperties;
  class Shader;
  class ShaderPipeline;
  class Texture;
  class GpuBuffer;
  class GpuQueryHeap;
  class GpuResource;
//---------------------------------------------------------------------------//  
  class RenderCore_Platform
  {
  public:
    RenderCore_Platform(RenderPlatformType aType) : myType(aType) {}
    virtual ~RenderCore_Platform() = default;

    virtual bool IsInitialized() = 0;
    virtual bool InitInternalResources() = 0;
    virtual void Shutdown() = 0;

    RenderPlatformType GetType() const { return myType; }
    const RenderPlatformCaps& GetCaps() const { return myCaps; }
    virtual RenderOutput* CreateRenderOutput(void* aNativeInstanceHandle, const WindowParameters& someWindowParams) = 0;
    virtual ShaderCompiler* CreateShaderCompiler() = 0;
    virtual Shader* CreateShader() = 0;
    virtual ShaderPipeline* CreateShaderPipeline() = 0;
    virtual Texture* CreateTexture() = 0;
    virtual GpuBuffer* CreateBuffer() = 0;
    virtual CommandList* CreateContext(CommandListType aType) = 0;
    virtual CommandQueue* CreateCommandQueue(CommandListType aType) = 0;
    virtual TextureView* CreateTextureView(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties, const char* aDebugName = nullptr) = 0;
    virtual GpuBufferView* CreateBufferView(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties, const char* aDebugName = nullptr) = 0;
    virtual GpuQueryHeap* CreateQueryHeap(GpuQueryType aType, uint aNumQueries) = 0;
    virtual uint GetQueryTypeDataSize(GpuQueryType aType) = 0;
    virtual float64 GetGpuTicksToMsFactor(CommandListType aCommandListType) = 0;
  protected:
    RenderPlatformCaps myCaps;
    RenderPlatformType myType;
  };
//---------------------------------------------------------------------------//
}
