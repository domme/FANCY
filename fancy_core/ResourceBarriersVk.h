#pragma once

#include "VkPrerequisites.h"
#include "RenderEnums.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct ResourceBarrierInfoVk
  {
    VkPipelineStageFlags myStageMask;
    VkAccessFlags myAccessMask;
    VkImageLayout myImageLayout;
  };
//---------------------------------------------------------------------------//
  namespace ResourceBarriersVk
  {
    ResourceBarrierInfoVk GetBarrierInfoVk(GpuResourceState aState, GpuResourceCategory aResourceCategory);
  }
//---------------------------------------------------------------------------//
}


