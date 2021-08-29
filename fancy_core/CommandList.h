#pragma once

#include "RendererPrerequisites.h"
#include "RenderCore.h"
#include "MathIncludes.h"
#include "RenderEnums.h"
#include "DataFormat.h"
#include "GpuBuffer.h"
#include "GpuResource.h"
#include "RaytracingShaderIdentifier.h"
#include "RaytracingShaderTable.h"
#include "VertexInputLayoutProperties.h"
#include "eastl/vector.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class GpuResourceView;
  class GpuFence;
  class GpuResource;
  class GpuResourceView;
  class DepthStencilState;
  class BlendState;
  class Shader;
  class GpuQueryHeap;
//---------------------------------------------------------------------------//
  struct GraphicsPipelineState
  {
    GraphicsPipelineState();
    uint64 GetHash() const;

    FillMode myFillMode;
    CullMode myCullMode;
    WindingOrder myWindingOrder;
    const DepthStencilState* myDepthStencilState;
    const BlendState* myBlendState;
    const ShaderPipeline* myShaderPipeline;
    const VertexInputLayout* myVertexInputLayout;
    uint8 myNumRenderTargets;
    DataFormat myRTVformats[RenderConstants::kMaxNumRenderTargets];
    DataFormat myDSVformat;
    TopologyType myTopologyType;
    
    bool myIsDirty : 1;
  };
//---------------------------------------------------------------------------//
  struct ComputePipelineState
  {
    ComputePipelineState();
    uint64 GetHash() const;

    const ShaderPipeline* myShaderPipeline;
    bool myIsDirty;
  };
//---------------------------------------------------------------------------//
  struct DispatchRaysDesc
  {
    RaytracingShaderTableRange myRayGenShaderTableRange;
    RaytracingShaderTableRange myMissShaderTableRange;
    RaytracingShaderTableRange myHitGroupTableRange;
    RaytracingShaderTableRange myCallableShaderTableRange;
    uint myWidth;
    uint myHeight;
    uint myDepth;
  };
//---------------------------------------------------------------------------//
  class CommandList
  {
    friend class CommandQueue;

  public:
    CommandList(CommandListType aType);
    virtual ~CommandList() {}

    CommandListType GetType() const { return myCommandListType; }

    virtual void ClearRenderTarget(TextureView* aTextureView, const float* aColor) = 0;
    virtual void ClearDepthStencilTarget(TextureView* aTextureView, float aDepthClear, uint8 aStencilClear, uint someClearFlags) = 0;
    virtual void CopyResource(GpuResource* aDstResource, GpuResource* aSrcResource) = 0;

    virtual void CopyBuffer(const GpuBuffer* aDstBuffer, uint64 aDstOffset, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset, uint64 aSize) = 0;

    void CopyTextureToBuffer(const GpuBuffer* aDstBuffer, uint64 aDstOffset, const Texture* aSrcTexture, const SubresourceLocation& aSrcSubresource);
    virtual void CopyTextureToBuffer(const GpuBuffer* aDstBuffer, uint64 aDstOffset, const Texture* aSrcTexture, const SubresourceLocation& aSrcSubresource, const TextureRegion& aSrcRegion) = 0;
    
    void CopyTexture(const Texture* aDstTexture, const SubresourceLocation& aDstSubresource, const Texture* aSrcTexture, const SubresourceLocation& aSrcSubresource);
    virtual void CopyTexture(const Texture* aDstTexture, const SubresourceLocation& aDstSubresource, const TextureRegion& aDstRegion, const Texture* aSrcTexture, const SubresourceLocation& aSrcSubresource, const TextureRegion& aSrcRegion) = 0;

    void CopyBufferToTexture(const Texture* aDstTexture, const SubresourceLocation& aDstSubresource, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset);
    virtual void CopyBufferToTexture(const Texture* aDstTexture, const SubresourceLocation& aDstSubresource, const TextureRegion& aDstRegion, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset) = 0;

    virtual void Dispatch(const glm::int3& aNumThreads) = 0;
    virtual void DispatchRays(const DispatchRaysDesc& aDesc) = 0;

    virtual void BindVertexBuffers(const GpuBuffer** someBuffers, uint64* someOffsets, uint64* someSizes, uint aNumBuffers) = 0;
    virtual void BindIndexBuffer(const GpuBuffer* aBuffer, uint anIndexSize, uint64 anOffset = 0u, uint64 aSize = ~0ULL) = 0;
    virtual void BindLocalBuffer(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someViewProperties, uint aRegisterIndex) = 0;
    
    virtual void Render(uint aNumIndicesPerInstance, uint aNumInstances, uint aStartIndex, uint aBaseVertex, uint aStartInstance) = 0;
    virtual void UpdateTextureData(const Texture* aDstTexture, const SubresourceRange& aSubresourceRange, const TextureSubData* someDatas, uint aNumDatas /*, const TextureRegion* someRegions = nullptr */) = 0; // TODO: Support regions

    virtual GpuQuery BeginQuery(GpuQueryType aType) = 0;
    virtual void EndQuery(const GpuQuery& aQuery) = 0;
    virtual GpuQuery InsertTimestamp() = 0;
    virtual void CopyQueryDataToBuffer(const GpuQueryHeap* aQueryHeap, const GpuBuffer* aBuffer, uint aFirstQueryIndex, uint aNumQueries, uint64 aBufferOffset) = 0;

    void TransitionResource(const GpuResource* aResource, ResourceTransition aTransition, uint someUsageFlags = 0u);
    virtual void TransitionResource(const GpuResource* aResource, const SubresourceRange& aSubresourceRange, ResourceTransition aTransition, uint someUsageFlags = 0u) = 0;

    void TransitionShaderResource(const GpuResourceView* aView, ShaderResourceTransition aTransition);
    virtual void TransitionShaderResource(const GpuResource* aResource, const SubresourceRange& aSubresourceRange, ShaderResourceTransition aTransition) = 0;

    void ResourceUAVbarrier(const GpuResource* aResource) { ResourceUAVbarrier(&aResource, 1u); }
    
    virtual void ResourceUAVbarrier(
      const GpuResource** someResources = nullptr, 
      uint aNumResources = 0u) = 0;

    virtual void Close() = 0;

    virtual void FlushBarriers() = 0;
    virtual void PostExecute(uint64 aFenceVal);
    virtual void ResetAndOpen();

    bool IsOpen() const { return myIsOpen; }
    void SetClipRect(const glm::uvec4& aRectangle); /// x, y, width, height
    const GpuBuffer* GetBuffer(uint64& anOffsetOut, GpuBufferUsage aType, const void* someData, uint64 aDataSize);
    const GpuBuffer* GetMappedBuffer(uint64& anOffsetOut, GpuBufferUsage aType, uint8** someDataPtrOut, uint64 aDataSize);
    void BindVertexBuffer(void* someData, uint64 aDataSize);
    void BindIndexBuffer(void* someData, uint64 aDataSize, uint anIndexSize);
    void BindConstantBuffer(void* someData, uint64 aDataSize, uint aRegisterIndex);
    void SetViewport(const glm::uvec4& uViewportParams); /// x, y, width, height
    const glm::uvec4& GetViewport() const { return myViewportParams; } /// x, y, width, height
    void SetShaderPipeline(const ShaderPipeline* aShaderPipeline);
    void SetBlendState(const SharedPtr<BlendState>& aBlendState);
    void SetDepthStencilState(const SharedPtr<DepthStencilState>& aDepthStencilState);
    void SetFillMode(const FillMode eFillMode);
    void SetCullMode(const CullMode eCullMode);
    void SetWindingOrder(const WindingOrder eWindingOrder);
    void SetTopologyType(TopologyType aType);
    void SetVertexInputLayout(const VertexInputLayout* anInputLayout);
    void SetRenderTarget(TextureView* aColorTarget, TextureView* aDepthStencil);
    void SetRenderTargets(TextureView** someColorTargets, uint aNumColorTargets, TextureView* aDepthStencil);
    void SetRaytracingPipelineState(RaytracingPipelineState* aPipelineState);
    void RemoveAllRenderTargets();
    void UpdateBufferData(const GpuBuffer* aDstBuffer, uint64 aDstOffset, const void* aDataPtr, uint64 aByteSize);
        
  protected:
    void ValidateTextureCopy(const TextureProperties& aDstProps, const SubresourceLocation& aDstSubresrource, const TextureRegion& aDstRegion,
      const TextureProperties& aSrcProps, const SubresourceLocation& aSrcSubresource, const TextureRegion& aSrcRegion) const;
    void ValidateTextureToBufferCopy(const GpuBufferProperties& aDstBufferProps, uint64 aDstBufferOffset, 
      const TextureProperties& aSrcTextureProps, const SubresourceLocation& aSrcSubresource, const TextureRegion& aSrcRegion) const;
    void ValidateBufferToTextureCopy(const TextureProperties& aDstTexProps, const SubresourceLocation& aDstSubresource,
                                     const TextureRegion& aDstRegion, const GpuBufferProperties& aSrcBufferProps, uint64 aSrcBufferOffset) const;
    void ValidateBufferCopy(const GpuBufferProperties& aDstProps, uint64 aDstOffset, const GpuBufferProperties& aSrcProps, uint64 aSrcOffset, uint64 aSize) const;

    GpuRingBuffer* GetUploadBuffer_Internal(uint64& anOffsetOut, GpuBufferUsage aType, const void* someData, uint64 aDataSize);

    enum Consts {
      kNumCachedBarriers = 256,
      kNumExpectedResourcesPerDispatch = 64,  // Mainly controls the hazard tracking data. TODO: Expose this in a better way - e.g. by making commandlists templated on the expected resource count
    };

    GpuQuery AllocateQuery(GpuQueryType aType);
    
    CommandListType myCommandListType;
    CommandListType myCurrentContext;

    glm::uvec4 myViewportParams;
    glm::uvec4 myClipRect;
    bool myIsOpen;
    bool myViewportDirty;
    bool myClipRectDirty;
    bool myTopologyDirty;
    bool myRenderTargetsDirty;
    TextureView* myRenderTargets[RenderConstants::kMaxNumRenderTargets];
    TextureView* myDepthStencilTarget;

    GraphicsPipelineState myGraphicsPipelineState;
    ComputePipelineState myComputePipelineState;
    RaytracingPipelineState* myRaytracingPipelineState;
    bool myRaytracingPipelineStateDirty;
    
    eastl::vector<GpuRingBuffer*> myUploadRingBuffers;
    eastl::vector<GpuRingBuffer*> myConstantRingBuffers;
    eastl::vector<GpuRingBuffer*> myVertexRingBuffers;
    eastl::vector<GpuRingBuffer*> myIndexRingBuffers;

    struct GpuQueryRange
    {
      uint myFirstQueryIdx = 0u;
      uint myNumQueries = 0u;
      uint myNumUsedQueries = 0u;
    };
    GpuQueryRange myQueryRanges[(uint)GpuQueryType::NUM];
  };
//---------------------------------------------------------------------------//
}
