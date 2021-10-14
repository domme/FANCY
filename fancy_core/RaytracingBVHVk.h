#pragma once
#include "RtAccelerationStructure.h"
#include "VkPrerequisites.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  class RaytracingBVHVk final : public RaytracingBVH
  {
  public:
    RaytracingBVHVk(const RaytracingAsProps& someProps, const eastl::span<RaytracingAsGeometryInfo>& someGeometries, const char* aName = nullptr);
    ~RaytracingBVHVk() override;
    void Destroy() override;

  private:
    VkAccelerationStructureKHR myAccelerationStructure = nullptr;
  };
}

#endif