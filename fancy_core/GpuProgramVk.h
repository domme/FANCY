#pragma once
#include "GpuProgram.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct GpuProgramCompilerOutput;
//---------------------------------------------------------------------------//
  class GpuProgramVk : public GpuProgram
  {
    friend class GpuProgramCompilerVk;

  public:
    GpuProgramVk();
    virtual ~GpuProgramVk() override;

    void SetFromCompilerOutput(const GpuProgramCompilerOutput& aCompilerOutput) override;
    uint64 GetNativeBytecodeHash() const override;
  };
//---------------------------------------------------------------------------//
}
