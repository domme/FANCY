#pragma once

#include "FancyCoreDefines.h"
#include "VkPrerequisites.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
 //---------------------------------------------------------------------------// 
  struct ShaderResourceInfoVk
  {
    bool operator==(const ShaderResourceInfoVk& anOther) const
    {
      return myDescriptorSet == anOther.myDescriptorSet &&
        myBindingInSet == anOther.myBindingInSet;
    }

    uint64 myNameHash = 0ull;
    eastl::string myName;
    VkDescriptorType myType = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    uint myDescriptorSet = 0u;
    uint myBindingInSet = 0u;
    uint myNumDescriptors = 0u;
  };
//---------------------------------------------------------------------------// 
}

#endif