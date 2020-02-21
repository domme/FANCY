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

  protected:
    VkSampler mySampler;
  };
//---------------------------------------------------------------------------//
}

