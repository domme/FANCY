#include "fancy_core_precompile.h"
#include "RtAccelerationStructure.h"

namespace Fancy
{
  RtAccelerationStructure::RtAccelerationStructure(RtAccelerationStructureType aType, const char* aName)
    : myType(aType)
    , myName(aName ? aName : "")
  {
  }
}
