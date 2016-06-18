#pragma once

#include "CommandContext.h"
#include <unordered_map>

namespace Fancy{
namespace Rendering{
class Descriptor;
}
}

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  struct ComputePipelineState
  {
    ComputePipelineState();
    uint GetHash();
    D3D12_COMPUTE_PIPELINE_STATE_DESC GetNativePSOdesc();

    const GpuProgram* myGpuProgram;
    bool myIsDirty;
  };
//---------------------------------------------------------------------------//
  class ComputeContextDX12 : public CommandContext
  {
  public:
    ComputeContextDX12();
    ~ComputeContextDX12() override;

    // Root arguments:
    void SetReadTexture(const Texture* aTexture, uint32 aRegisterIndex) const;
    void SetWriteTexture(const Texture* aTexture, uint32 aRegisterIndex) const;
    void SetReadBuffer(const GpuBuffer* aBuffer, uint32 aRegisterIndex) const;
    void SetConstantBuffer(const GpuBuffer* aConstantBuffer, uint32 aRegisterIndex) const;

    // Descriptor tables:
    void SetMultipleResources(const Descriptor* someResources, uint32 aResourceCount, uint32 aRegisterIndex);

    void SetComputeProgram(const GpuProgram* aProgram);
    void Dispatch(size_t GroupCountX, size_t GroupCountY, size_t GroupCountZ);
    
  protected:
    static std::unordered_map<uint, ID3D12PipelineState*> ourPSOcache;

    ComputePipelineState myComputePipelineState;

    void ResetInternal() override;

    void ApplyPipelineState();
  };
//---------------------------------------------------------------------------//
} } }
