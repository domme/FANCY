#pragma once

#include "RendererPrerequisites.h"
#include "CommandContext.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class DepthStencilState;
  class BlendState;
  class Descriptor;
//---------------------------------------------------------------------------//
  struct GraphicsPipelineState
  {
    GraphicsPipelineState();
    uint GetHash();
    
    FillMode myFillMode;
    CullMode myCullMode;
    WindingOrder myWindingOrder;
    SharedPtr<DepthStencilState> myDepthStencilState;
    SharedPtr<BlendState> myBlendState;
    SharedPtr<GpuProgramPipeline> myGpuProgramPipeline;
    uint8 myNumRenderTargets;
    DataFormat myRTVformats[Constants::kMaxNumRenderTargets];
    DataFormat myDSVformat;

    bool myIsDirty : 1;
  };
//---------------------------------------------------------------------------//
  class RenderContext : public CommandContext
  {
  public:
    RenderContext();
    virtual ~RenderContext() {}

    virtual void SetGpuProgramPipeline(const SharedPtr<GpuProgramPipeline>& aGpuProgramPipeline);
    
    virtual void SetVertexIndexBuffers(const Rendering::GpuBuffer* aVertexBuffer, const Rendering::GpuBuffer* anIndexBuffer,
      uint aVertexOffset = 0u, uint aNumVertices = UINT_MAX, uint anIndexOffset = 0u, uint aNumIndices = UINT_MAX) = 0;
    virtual void Render(uint aNumIndicesPerInstance, uint aNumInstances, uint anIndexOffset, uint aVertexOffset, uint anInstanceOffset) = 0;

    virtual void RenderGeometry(const Geometry::GeometryData* pGeometry) = 0;

    virtual void SetClipRect(const glm::uvec4& aRectangle); /// x, y, width, height
    void SetViewport(const glm::uvec4& uViewportParams); /// x, y, width, height
    const glm::uvec4& GetViewport() const { return myViewportParams; } /// x, y, width, height
    void SetBlendState(const SharedPtr<BlendState>& aBlendState);
    void SetDepthStencilState(const SharedPtr<DepthStencilState>& aDepthStencilState);
    void SetFillMode(const FillMode eFillMode);
    void SetCullMode(const CullMode eCullMode);
    void SetWindingOrder(const WindingOrder eWindingOrder);

    void SetDepthStencilRenderTarget(Texture* pDStexture);
    void SetRenderTarget(Texture* pRTTexture, const uint8 u8RenderTargetIndex);
    void RemoveAllRenderTargets();

  protected:
    GraphicsPipelineState myGraphicsPipelineState;

    glm::uvec4 myViewportParams;
    glm::uvec4 myClipRect;
    bool myViewportDirty;
    bool myClipRectDirty;

    Texture* myRenderTargets[Rendering::Constants::kMaxNumRenderTargets];
    Texture* myDepthStencilTarget;
    bool myRenderTargetsDirty;
  };
//---------------------------------------------------------------------------//
} }
