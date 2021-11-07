#pragma once
#include "RtAccelerationStructure.h"
#include "VkPrerequisites.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  class RtAccelerationStructureVk final : public RtAccelerationStructure
  {
  public:
    RtAccelerationStructureVk(const RtAccelerationStructureGeometryData* someGeometries, uint aNumGeometries, uint someFlags = 0, const char* aName = nullptr);
    RtAccelerationStructureVk(const RtAccelerationStructureInstanceData* someInstances, uint aNumInstances, uint someFlags = 0, const char* aName = nullptr);
    ~RtAccelerationStructureVk() override;

  private:
    void BuildInternal(
      eastl::vector<VkAccelerationStructureGeometryKHR>* someGeometryDescs,
      eastl::vector<uint>* somePrimitiveCounts,
      eastl::vector<VkAccelerationStructureInstanceKHR>* someInstanceDescs, 
      uint someFlags, const char* aName, CommandList* cmdList);

    void Destroy();
    
    VkAccelerationStructureKHR myAccelerationStructure = nullptr;
  };
}

#endif