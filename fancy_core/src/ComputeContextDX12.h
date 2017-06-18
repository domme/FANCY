#pragma once

#include "ComputeContext.h"
#include "CommandContextDX12.h"

#include <unordered_map>

namespace Fancy { namespace Rendering {
  class Descriptor;
} }

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class ComputeContextDX12 : public ComputeContext, public CommandContextDX12
  {
  public:
    ComputeContextDX12();
    ~ComputeContextDX12() override;

    void Reset() override;
    uint64 ExecuteAndReset(bool aWaitForCompletion) override;

    static D3D12_COMPUTE_PIPELINE_STATE_DESC GetNativePSOdesc(const ComputePipelineState& aState);

    void ClearRenderTarget(Texture* aTexture, const float* aColor) override;
    void ClearDepthStencilTarget(Texture* aTexture, float aDepthClear, uint8 aStencilClear, uint32 someClearFlags = (uint32)DepthStencilClearFlags::CLEAR_ALL) const override;

    // Root arguments:
    void BindResource(const GpuResource* aResource, ResourceBindingType aBindingType, uint32 aRegisterIndex) const override;

    // Descriptor tables:
    void BindResourceSet(const GpuResource** someResources, ResourceBindingType* someBindingTypes, uint32 aResourceCount, uint32 aRegisterIndex) override;

    void SetComputeProgram(const GpuProgram* aProgram) override;
    void Dispatch(size_t GroupCountX, size_t GroupCountY, size_t GroupCountZ) override;
    
  protected:
    void Reset_Internal() override;
    void ApplyPipelineState();

    static std::unordered_map<uint, ID3D12PipelineState*> ourPSOcache;
    ComputePipelineState myComputePipelineState;
  };
//---------------------------------------------------------------------------//
} } }
