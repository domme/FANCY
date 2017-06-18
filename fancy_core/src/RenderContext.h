#pragma once

#include "RendererPrerequisites.h"
#include "CommandContext.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct DepthStencilState;
  struct BlendState;
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
  class RenderContext : public virtual CommandContext
  {
  public:
    RenderContext();
    virtual ~RenderContext() {}

    virtual void SetGpuProgramPipeline(const SharedPtr<GpuProgramPipeline>& aGpuProgramPipeline);

    virtual void RenderGeometry(const Geometry::GeometryData* pGeometry) = 0;

    void SetViewport(const glm::uvec4& uViewportParams); /// x, y, width, height
    const glm::uvec4& GetViewport() const { return myViewportParams; } /// x, y, width, height
    void SetBlendState(std::shared_ptr<BlendState> aBlendState);
    void SetDepthStencilState(std::shared_ptr<DepthStencilState> aDepthStencilState);
    void SetFillMode(const FillMode eFillMode);
    void SetCullMode(const CullMode eCullMode);
    void SetWindingOrder(const WindingOrder eWindingOrder);

    void SetDepthStencilRenderTarget(Texture* pDStexture);
    void SetRenderTarget(Texture* pRTTexture, const uint8 u8RenderTargetIndex);
    void RemoveAllRenderTargets();

  protected:
    GraphicsPipelineState myGraphicsPipelineState;

    glm::uvec4 myViewportParams;
    bool myViewportDirty;

    Texture* myRenderTargets[Rendering::Constants::kMaxNumRenderTargets];
    Texture* myDepthStencilTarget;
    bool myRenderTargetsDirty;
  };
//---------------------------------------------------------------------------//
} }
