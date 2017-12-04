#pragma once

#include <unordered_map>

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "DX12Prerequisites.h"
#include "RenderContext.h"
#include "CommandContextDX12.h"

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
  class DLLEXPORT RenderContextDX12 : public RenderContext, public CommandContextDX12
  {
  public:
    RenderContextDX12();
    ~RenderContextDX12() override;

    void Reset() override;
    uint64 ExecuteAndReset(bool aWaitForCompletion) override;

    static D3D12_GRAPHICS_PIPELINE_STATE_DESC GetNativePSOdesc(const GraphicsPipelineState& aState);

    void ClearRenderTarget(Texture* aTexture, const float* aColor) override;
    void ClearDepthStencilTarget(Texture* aTexture, float aDepthClear, uint8 aStencilClear, uint32 someClearFlags = (uint32)DepthStencilClearFlags::CLEAR_ALL) override;

    // Root arguments:
    void BindResource(const GpuResource* aResource, DescriptorType aBindingType, uint32 aRegisterIndex) const override;

    // Descriptor tables:
    void BindDescriptorSet(const Descriptor** someDescriptors, uint32 aResourceCount, uint32 aRegisterIndex) override;

    void TransitionResource(GpuResource* aResource, GpuResourceState aTransitionToState, bool aKickoffNow = false) override;

    void SetGpuProgramPipeline(const SharedPtr<GpuProgramPipeline>& aGpuProgramPipeline) override;

    void SetVertexIndexBuffers(const Rendering::GpuBuffer* aVertexBuffer, const Rendering::GpuBuffer* anIndexBuffer,
      uint aVertexOffset = 0u, uint aNumVertices = UINT_MAX, uint anIndexOffset = 0u, uint aNumIndices = UINT_MAX) override;
    void Render(uint aNumIndicesPerInstance, uint aNumInstances, uint anIndexOffset, uint aVertexOffset, uint anInstanceOffset) override;

    void RenderGeometry(const Geometry::GeometryData* pGeometry) override;
  
  protected:
    static std::unordered_map<uint, ID3D12PipelineState*> ourPSOcache;

    void Reset_Internal() override;

    void ApplyViewportAndClipRect();
    void ApplyPipelineState();
    void ApplyRenderTargets();
  };
//---------------------------------------------------------------------------//
} } }
