#pragma once

#if FANCY_ENABLE_VK

#include "Rendering/GlobalDescriptorAllocation.h"
#include "Rendering/TextureSampler.h"
#include "VkPrerequisites.h"

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