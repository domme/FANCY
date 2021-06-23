#pragma once

#include "GlobalDescriptorAllocation.h"
#include "VkPrerequisites.h"
#include "TextureSampler.h"

#if FANCY_ENABLE_VK

namespace Fancy {
//---------------------------------------------------------------------------//
  class TextureSamplerVk : public TextureSampler
  {
  public:
    TextureSamplerVk(const TextureSamplerProperties& someProperties);
    ~TextureSamplerVk() override;

    VkSampler GetVkSampler() const { return mySampler; }

  protected:
    VkSampler mySampler;
    GlobalDescriptorAllocation myDescriptorAllocation;
  };
//---------------------------------------------------------------------------//
}

#endif