#pragma once

#include "RendererPrerequisites.h"
#include "CommandContext.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct ComputePipelineState
  {
    ComputePipelineState();
    uint GetHash() const;
    
    const GpuProgram* myGpuProgram;
    bool myIsDirty;
  };
//---------------------------------------------------------------------------//
  class ComputeContext : public virtual CommandContext
  {
  public:
    ComputeContext();
    virtual ~ComputeContext();

    virtual void SetComputeProgram(const GpuProgram* aProgram) = 0;
    virtual void Dispatch(size_t GroupCountX, size_t GroupCountY, size_t GroupCountZ) = 0;
  };
//---------------------------------------------------------------------------//
} }