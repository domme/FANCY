#pragma once

#include "RendererPrerequisites.h"
#include "CommandListType.h"
#include "Descriptor.h"

namespace Fancy {
  class GpuFence;
  //---------------------------------------------------------------------------//
  class Descriptor;
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

    virtual void ClearRenderTarget(Texture* aTexture, const float* aColor) = 0;
    virtual void ClearDepthStencilTarget(Texture* aTexture, float aDepthClear, uint8 aStencilClear, uint someClearFlags = (uint)DepthStencilClearFlags::CLEAR_ALL) = 0;
    virtual void CopyResource(GpuResource* aDestResource, GpuResource* aSrcResource) = 0;
    virtual uint64 ExecuteAndReset(bool aWaitForCompletion = false) = 0;
    virtual void Dispatch(uint GroupCountX, uint GroupCountY, uint GroupCountZ) = 0;
    virtual void BindResource(const GpuResource* aResource, DescriptorType aBindingType, uint aRegisterIndex) const = 0;
    virtual void BindDescriptorSet(const Descriptor** someDescriptors, uint aResourceCount, uint aRegisterIndex) = 0;
    virtual void SetVertexIndexBuffers(const GpuBuffer* aVertexBuffer, const GpuBuffer* anIndexBuffer, uint aVertexOffset = 0u, uint aNumVertices = UINT_MAX, uint anIndexOffset = 0u, uint aNumIndices = UINT_MAX) = 0;
    virtual void Render(uint aNumIndicesPerInstance, uint aNumInstances, uint anIndexOffset, uint aVertexOffset, uint anInstanceOffset) = 0;
    virtual void RenderGeometry(const GeometryData* pGeometry) = 0;
    virtual void TransitionResourceList(GpuResource** someResources, GpuResourceState* someTransitionToStates, uint aNumResources) = 0;
    virtual uint64 SignalFence(GpuFence* aFence) = 0;
    
    virtual void SetGpuProgramPipeline(const SharedPtr<GpuProgramPipeline>& aGpuProgramPipeline);
    virtual void SetComputeProgram(const GpuProgram* aProgram);
    virtual void SetClipRect(const glm::uvec4& aRectangle); /// x, y, width, height
    virtual void Reset();

    void TransitionResource(GpuResource* aResource, GpuResourceState aTransitionToState);
    void TransitionResource(GpuResource* aResource1, GpuResourceState aTransitionToState1,
      GpuResource* aResource2, GpuResourceState aTransitionToState2);
    void TransitionResource(GpuResource* aResource1, GpuResourceState aTransitionToState1,
      GpuResource* aResource2, GpuResourceState aTransitionToState2,
      GpuResource* aResource3, GpuResourceState aTransitionToState3);
    void TransitionResource(GpuResource* aResource1, GpuResourceState aTransitionToState1,
      GpuResource* aResource2, GpuResourceState aTransitionToState2,
      GpuResource* aResource3, GpuResourceState aTransitionToState3,
      GpuResource* aResource4, GpuResourceState aTransitionToState4);
    
    void SetViewport(const glm::uvec4& uViewportParams); /// x, y, width, height
    const glm::uvec4& GetViewport() const { return myViewportParams; } /// x, y, width, height
    void SetBlendState(const SharedPtr<BlendState>& aBlendState);
    void SetDepthStencilState(const SharedPtr<DepthStencilState>& aDepthStencilState);
    void SetFillMode(const FillMode eFillMode);
    void SetCullMode(const CullMode eCullMode);
    void SetWindingOrder(const WindingOrder eWindingOrder);
    void SetTopologyType(TopologyType aType);
    void SetDepthStencilRenderTarget(Texture* pDStexture);
    void SetRenderTarget(Texture* pRTTexture, const uint8 u8RenderTargetIndex);
    void RemoveAllRenderTargets();
    
  protected:
    CommandListType myCommandListType;

    GraphicsPipelineState myGraphicsPipelineState;
    ComputePipelineState myComputePipelineState;

    glm::uvec4 myViewportParams;
    glm::uvec4 myClipRect;
    bool myViewportDirty;
    bool myClipRectDirty;
    bool myTopologyDirty;
    Texture* myRenderTargets[Constants::kMaxNumRenderTargets];
    Texture* myDepthStencilTarget;
    bool myRenderTargetsDirty;
  };
//---------------------------------------------------------------------------//
}
