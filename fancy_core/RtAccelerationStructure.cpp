#include "fancy_core_precompile.h"
#include "RtAccelerationStructure.h"

namespace Fancy
{
  RtAccelerationStructure::RtAccelerationStructure(const RtAccelerationStructureGeometryData* /*someGeometries*/, uint /*aNumGeometries*/, uint /*aSomeFlags*/, const char* /*aName*/)
      : myType(RtAccelerationStructureType::BOTTOM_LEVEL)
  {
  }

  RtAccelerationStructure::RtAccelerationStructure(const RtAccelerationStructureInstanceData* /*someInstances*/, uint /*aNumInstances*/, uint /*someFlags*/, const char* /*aName*/)
    : myType(RtAccelerationStructureType::TOP_LEVEL)
  {
  }
}
