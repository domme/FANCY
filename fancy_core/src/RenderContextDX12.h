#pragma once

#include <unordered_map>

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "CommandContext.h"

namespace Fancy{ namespace Rendering{
  class Descriptor;
  class GpuResource;
  class RenderOutput;
  class GpuProgramPipeline;
  class BlendState;
  class DepthStencilState;
}}

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class DescriptorHeapDX12;
  class GpuResourceDX12;
  class CommandAllocatorPoolDX12;
//---------------------------------------------------------------------------//
  struct GraphicsPipelineState
  {
    GraphicsPipelineState();
    uint getHash();
    D3D12_GRAPHICS_PIPELINE_STATE_DESC GetNativePSOdesc();

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
  class RenderContextDX12 : public CommandContext
  {
  public:
     RenderContextDX12();
     ~RenderContextDX12() override;

    // Root arguments:
    void SetReadTexture(const Texture* aTexture, uint32 aRegisterIndex) const;
    void SetWriteTexture(const Texture* aTexture, uint32 aRegisterIndex) const;
    void SetReadBuffer(const GpuBuffer* aBuffer, uint32 aRegisterIndex) const;
    void SetConstantBuffer(const GpuBuffer* aConstantBuffer, uint32 aRegisterIndex) const;
    
    // Descriptor tables:
    void SetMultipleResources(const Descriptor* someResources, uint32 aResourceCount, uint32 aRegisterIndex);
    
    void SetGpuProgramPipeline(const SharedPtr<GpuProgramPipeline>& pProgramPipeline);

    // It might be ok to keep these state-modifiers the way they are for a more modern approach
    void setViewport(const glm::uvec4& uViewportParams); /// x, y, width, height
    const glm::uvec4 getViewport() const { return myViewportParams; } /// x, y, width, height
    void SetBlendState(std::shared_ptr<BlendState> aBlendState);
    void SetDepthStencilState(std::shared_ptr<DepthStencilState> aDepthStencilState);
    void setFillMode(const FillMode eFillMode);
    void setCullMode(const CullMode eCullMode);
    void setWindingOrder(const WindingOrder eWindingOrder);

    void setDepthStencilRenderTarget(Texture* pDStexture);
    void setRenderTarget(Texture* pRTTexture, const uint8 u8RenderTargetIndex);
    void removeAllRenderTargets();

    void renderGeometry(const Geometry::GeometryData* pGeometry);

  protected:
    static std::unordered_map<uint, ID3D12PipelineState*> ourPSOcache;

    void ResetInternal() override;

    void ApplyViewport();
    void ApplyPipelineState();
    void ApplyRenderTargets();
    
    GraphicsPipelineState myGraphicsPipelineState;

    glm::uvec4 myViewportParams;
    bool myViewportDirty;

    Texture* myRenderTargets[Rendering::Constants::kMaxNumRenderTargets];
    Texture* myDepthStencilTarget;
    bool myRenderTargetsDirty;

    
  };
//---------------------------------------------------------------------------//
} } }
