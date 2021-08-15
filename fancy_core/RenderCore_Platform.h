#pragma once

#include "RendererPrerequisites.h"
#include "Ptr.h"
#include "EASTL/span.h"

namespace Fancy {
  //---------------------------------------------------------------------------//
  class RaytracingShaderTable;
  struct RaytracingShaderTableProperties;
  class RaytracingPipelineState;
  class RaytracingPipelineStateProperties;
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
  class TextureSampler;
  struct TextureSamplerProperties;
  struct GpuResourceViewRange;
  class GpuResourceView;
  struct RaytracingBVHGeometry;
  class RaytracingBVH;
  struct RaytracingBVHProps;
//---------------------------------------------------------------------------//
  
  class RenderCore_Platform
  {
  public:
    RenderCore_Platform(RenderPlatformType aType, const RenderPlatformProperties& someProperties)
    : myProperties(someProperties)
    , myType(aType) {}

    virtual ~RenderCore_Platform() = default;

    virtual bool IsInitialized() = 0;
    virtual bool InitInternalResources() = 0;
    virtual void Shutdown() = 0;
    virtual void BeginFrame() {}
    virtual void EndFrame() {}

    RenderPlatformType GetType() const { return myType; }
    const RenderPlatformProperties& GetProperties() const { return myProperties; }
    const RenderPlatformCaps& GetCaps() const { return myCaps; }
    virtual RenderOutput* CreateRenderOutput(void* aNativeInstanceHandle, const WindowParameters& someWindowParams) = 0;
    virtual ShaderCompiler* CreateShaderCompiler() = 0;
    virtual Shader* CreateShader() = 0;
    virtual ShaderPipeline* CreateShaderPipeline() = 0;
    virtual Texture* CreateTexture() = 0;
    virtual GpuBuffer* CreateBuffer() = 0;
    virtual TextureSampler* CreateTextureSampler(const TextureSamplerProperties& someProperties) = 0;
    virtual CommandList* CreateCommandList(CommandListType aType) = 0;
    virtual CommandQueue* CreateCommandQueue(CommandListType aType) = 0;
    virtual TextureView* CreateTextureView(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties, const char* aDebugName = nullptr) = 0;
    virtual GpuBufferView* CreateBufferView(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties, const char* aDebugName = nullptr) = 0;
    virtual RaytracingBVH* CreateRtAccelerationStructure(const RaytracingBVHProps& someProps, const eastl::span<RaytracingBVHGeometry>& someGeometries, const char* aName = nullptr) = 0;
    virtual GpuQueryHeap* CreateQueryHeap(GpuQueryType aType, uint aNumQueries) = 0;
    virtual uint GetQueryTypeDataSize(GpuQueryType aType) = 0;
    virtual float64 GetGpuTicksToMsFactor(CommandListType aCommandListType) = 0;
    virtual RaytracingPipelineState* CreateRtPipelineState(const RaytracingPipelineStateProperties& someProps) = 0;
    virtual RaytracingShaderTable* CreateRtShaderTable(const RaytracingShaderTableProperties& someProps, const SharedPtr<RaytracingPipelineState>& anRtPso) = 0;

  protected:
    RenderPlatformCaps myCaps;
    RenderPlatformProperties myProperties;
    RenderPlatformType myType;
  };
//---------------------------------------------------------------------------//
}
