#pragma once
#include "RaytracingBVH.h"
#include "VkPrerequisites.h"

namespace Fancy
{
  class RaytracingBVHVk final : public RaytracingBVH
  {
  public:
    RaytracingBVHVk(const RaytracingBVHProps& someProps, const eastl::span<RaytracingBVHGeometry>& someGeometries, const char* aName = nullptr);
    ~RaytracingBVHVk() override;
    void Destroy() override;

  private:
    VkAccelerationStructureKHR myAccelerationStructure = nullptr;
  };
}
