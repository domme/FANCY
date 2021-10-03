#pragma once
#include "RaytracingAS.h"
#include "FancyCoreDefines.h"
#include "DX12Prerequisites.h"
#include "EASTL/span.h"
#include "Ptr.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
  class GpuBuffer;

  class RaytracingAsDX12 : public RaytracingAS
  {
  public:
    RaytracingAsDX12(const RaytracingAsProps& someProps, const eastl::span<RaytracingAsGeometryInfo>& someGeometries, const char* aName = nullptr);
    void Destroy() override;

  private:
    SharedPtr<GpuBuffer> myAccelerationStructureBuffer;
  };
}

#endif

