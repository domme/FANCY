#pragma once
#include "RtAccelerationStructure.h"
#include "FancyCoreDefines.h"
#include "DX12Prerequisites.h"
#include "Ptr.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
  class GpuBuffer;

  class RtAccelerationStructureDX12 : public RtAccelerationStructure
  {
  public:
    RtAccelerationStructureDX12(const RtAccelerationStructureGeometryData* someGeometries, uint aNumGeometries, uint someFlags = 0, const char* aName = nullptr);
    RtAccelerationStructureDX12(const RtAccelerationStructureInstanceData* someInstances, uint aNumInstances, uint someFlags = 0, const char* aName = nullptr);
    ~RtAccelerationStructureDX12() override;
  };
}

#endif

