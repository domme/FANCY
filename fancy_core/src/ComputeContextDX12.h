#pragma once

#include "ComputeContext.h"
#include "CommandContextBaseDX12.h"

#include <unordered_map>

namespace Fancy { namespace Rendering {
  class Descriptor;
} }

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class ComputeContextDX12 : public ComputeContext, public CommandContextBaseDX12
  {
  public:
    ComputeContextDX12();
    ~ComputeContextDX12() override;

    static D3D12_COMPUTE_PIPELINE_STATE_DESC GetNativePSOdesc(const ComputePipelineState& aState);

    // Root arguments:
    void SetReadTexture(const Texture* aTexture, uint32 aRegisterIndex) const override;
    void SetWriteTexture(const Texture* aTexture, uint32 aRegisterIndex) const override;
    void SetReadBuffer(const GpuBuffer* aBuffer, uint32 aRegisterIndex) const override;
    void SetConstantBuffer(const GpuBuffer* aConstantBuffer, uint32 aRegisterIndex) const override;

    // Descriptor tables:
    void SetMultipleResources(const Descriptor* someResources, uint32 aResourceCount, uint32 aRegisterIndex) override;

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
