#include "ComputeContext.h"
#include "MathUtil.h"
#include "GpuProgram.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  ComputePipelineState::ComputePipelineState()
    : myIsDirty(true)
    , myGpuProgram(nullptr)
  {
  }
  //---------------------------------------------------------------------------//
  uint ComputePipelineState::GetHash() const
  {
    uint hash = 0u;
    MathUtil::hash_combine(hash, reinterpret_cast<uint64>(myGpuProgram));

    if (myGpuProgram != nullptr)
      MathUtil::hash_combine(hash, myGpuProgram->GetNativeBytecodeHash());

    return hash;
  }
//---------------------------------------------------------------------------//
} }
