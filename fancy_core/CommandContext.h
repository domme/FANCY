#pragma once

#include "RendererPrerequisites.h"
#include "CommandListType.h"
#include "RenderCore.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class GpuResourceView;
  class GpuFence;
  class GpuResource;  
  class DepthStencilState;
  class BlendState;
//---------------------------------------------------------------------------//
  struct GraphicsPipelineState
  {
    GraphicsPipelineState();
    uint64 GetHash() const;

    FillMode myFillMode;
    CullMode myCullMode;
    WindingOrder myWindingOrder;
    SharedPtr<DepthStencilState> myDepthStencilState;
    SharedPtr<BlendState> myBlendState;
    SharedPtr<GpuProgramPipeline> myGpuProgramPipeline;
    uint8 myNumRenderTargets;
    DataFormat myRTVformats[Constants::kMaxNumRenderTargets];
    DataFormat myDSVformat;
    TopologyType myTopologyType;

    bool myIsDirty : 1;
  };
//---------------------------------------------------------------------------//
  struct ComputePipelineState
  {
    ComputePipelineState();
    uint64 GetHash() const;

    const GpuProgram* myGpuProgram;
    bool myIsDirty;
  };
//---------------------------------------------------------------------------//
  class CommandContext
  {
  public:
    CommandContext(CommandListType aType);
    virtual ~CommandContext() {}

    CommandListType GetType() const { return myCommandListType; }

    virtual void ClearRenderTarget(TextureView* aTextureView, const float* aColor) = 0;
    virtual void ClearDepthStencilTarget(TextureView* aTextureView, float aDepthClear, uint8 aStencilClear, uint someClearFlags = (uint)DepthStencilClearFlags::CLEAR_ALL) = 0;
    virtual void CopyResource(GpuResource* aDestResource, GpuResource* aSrcResource) = 0;
    virtual void CopyBufferRegion(const GpuBuffer* aDestBuffer, uint64 aDestOffset, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset, uint64 aSize) = 0;
    virtual void CopyTextureRegion(const Texture* aDestTexture, const TextureSubLocation& aDestSubLocation, const glm::uvec3& aDestTexelPos, const Texture* aSrcTexture, const TextureSubLocation& aSrcSubLocation, const TextureRegion* aSrcRegion = nullptr) = 0;
    virtual void CopyTextureRegion(const Texture* aDestTexture, const TextureSubLocation& aDestSubLocation, const glm::uvec3& aDestTexelPos, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset) = 0;
    virtual void Dispatch(const glm::int3& aNumThreads) = 0;
    virtual void BindBuffer(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someViewProperties, uint aRegisterIndex) const = 0;
    virtual void BindResourceSet(const GpuResourceView** someResourceViews, uint aResourceCount, uint aRegisterIndex) = 0;
    virtual void BindVertexBuffer(const GpuBuffer* aBuffer, uint aVertexSize, uint64 anOffset = 0u, uint64 aSize = ~0ULL) = 0;
    virtual void BindIndexBuffer(const GpuBuffer* aBuffer, uint anIndexSize, uint64 anOffset = 0u, uint64 aSize = ~0ULL) = 0;
    virtual void Render(uint aNumIndicesPerInstance, uint aNumInstances, uint aStartIndex, uint aBaseVertex, uint aStartInstance) = 0;
    virtual void RenderGeometry(const GeometryData* pGeometry) = 0;
    virtual void TransitionResourceList(const GpuResource** someResources, GpuResourceTransition* someTransitions, uint aNumResources) = 0;
    
    virtual void SetGpuProgramPipeline(const SharedPtr<GpuProgramPipeline>& aGpuProgramPipeline);
    virtual void SetComputeProgram(const GpuProgram* aProgram);
    virtual void SetClipRect(const glm::uvec4& aRectangle); /// x, y, width, height
    virtual void Reset(uint64 aFenceVal);

    const GpuBuffer* GetBuffer(uint64& anOffsetOut, GpuBufferUsage aType, const void* someData, uint64 aDataSize);
    void BindVertexBuffer(void* someData, uint64 aDataSize, uint aVertexSize);
    void BindIndexBuffer(void* someData, uint64 aDataSize, uint anIndexSize);
    void BindConstantBuffer(void* someData, uint64 aDataSize, uint aRegisterIndex);
    void SetViewport(const glm::uvec4& uViewportParams); /// x, y, width, height
    const glm::uvec4& GetViewport() const { return myViewportParams; } /// x, y, width, height
    void SetBlendState(const SharedPtr<BlendState>& aBlendState);
    void SetDepthStencilState(const SharedPtr<DepthStencilState>& aDepthStencilState);
    void SetFillMode(const FillMode eFillMode);
    void SetCullMode(const CullMode eCullMode);
    void SetWindingOrder(const WindingOrder eWindingOrder);
    void SetTopologyType(TopologyType aType);
    void SetRenderTarget(TextureView* aColorTarget, TextureView* aDepthStencil);
    void SetRenderTargets(TextureView** someColorTargets, uint aNumColorTargets, TextureView* aDepthStencil);

    void RemoveAllRenderTargets();

    void UpdateBufferData(const GpuBuffer* aDestBuffer, uint64 aDestOffset, const void* aDataPtr, uint64 aByteSize);
    void UpdateTextureData(const Texture* aDestTexture, const TextureSubLocation& aStartSubLocation, const TextureSubData* someDatas, uint aNumDatas /*, const TextureRegion* someRegions = nullptr */); // TODO: Support regions

    void* ReadbackBufferData(const GpuBuffer* aBuffer, uint64 anOffset, uint64 aByteSize);

    void TransitionResource(const GpuResource* aResource, GpuResourceTransition aTransition);
    void TransitionResource(const GpuResource* aResource1, GpuResourceTransition aTransition1,
                            const GpuResource* aResource2, GpuResourceTransition aTransition2);
    void TransitionResource(const GpuResource* aResource1, GpuResourceTransition aTransition1,
      const GpuResource* aResource2, GpuResourceTransition aTransition2,
      const GpuResource* aResource3, GpuResourceTransition aTransition3);
    void TransitionResource(const GpuResource* aResource1, GpuResourceTransition aTransition1,
      const GpuResource* aResource2, GpuResourceTransition aTransition2,
      const GpuResource* aResource3, GpuResourceTransition aTransition3,
      const GpuResource* aResource4, GpuResourceTransition aTransition4);
    
  protected:
    CommandListType myCommandListType;
    CommandListType myCurrentContext;

    glm::uvec4 myViewportParams;
    glm::uvec4 myClipRect;
    bool myViewportDirty;
    bool myClipRectDirty;
    bool myTopologyDirty;
    bool myRenderTargetsDirty;
    bool myShaderHasUnorderedWrites;
    TextureView* myRenderTargets[Constants::kMaxNumRenderTargets];
    TextureView* myDepthStencilTarget;

    GraphicsPipelineState myGraphicsPipelineState;
    ComputePipelineState myComputePipelineState;

    GpuBuffer* myReadbackBuffer;
    DynamicArray<GpuRingBuffer*> myUploadRingBuffers;
    DynamicArray<GpuRingBuffer*> myConstantRingBuffers;
    DynamicArray<GpuRingBuffer*> myVertexRingBuffers;
    DynamicArray<GpuRingBuffer*> myIndexRingBuffers;
  };
//---------------------------------------------------------------------------//
}
