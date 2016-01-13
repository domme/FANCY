#include "GpuProgram.h"
#include "MathUtil.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  GpuProgramDesc GpuProgram::GetDescription() const
  {
      
  }
//---------------------------------------------------------------------------//
  bool GpuProgram::operator==(const GpuProgramDesc& anOtherDesc) const 
  {
    const GpuProgramDesc& desc = GetDescription();
    return MathUtil::hashFromGeneric(desc) == MathUtil::hashFromGeneric(anOtherDesc);
  }
//---------------------------------------------------------------------------//
} }