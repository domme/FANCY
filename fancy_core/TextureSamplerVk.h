#pragma once

#include "VkPrerequisites.h"
#include "TextureSampler.h"

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
  };
//---------------------------------------------------------------------------//
}

