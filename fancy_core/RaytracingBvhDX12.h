#pragma once
#include "RaytracingBVH.h"
#include "FancyCoreDefines.h"
#include "DX12Prerequisites.h"
#include "EASTL/span.h"
#include "Ptr.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
  class GpuBuffer;

  class RaytracingBvhDX12 : public RaytracingBVH
  {
  public:
    RaytracingBvhDX12(const RaytracingBVHProps& someProps, const eastl::span<RaytracingBVHGeometry>& someGeometries, const char* aName = nullptr);
    ~RaytracingBvhDX12() override;
    void Destroy() override;

  private:
    SharedPtr<GpuBuffer> myAccelerationStructureBuffer;

  };
}

#endif

