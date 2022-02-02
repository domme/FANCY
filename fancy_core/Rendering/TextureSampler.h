#pragma once

#include "TextureSamplerProperties.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class TextureSampler
  {
  public:
    virtual ~TextureSampler() = default;

    const TextureSamplerProperties GetProperties() const { return myProperties; }
    uint GetGlobalDescriptorIndex() const { return myGlobalDescriptorIndex; }

  protected:
    TextureSampler(const TextureSamplerProperties& someProperties)
      : myProperties(someProperties)
      , myGlobalDescriptorIndex(UINT_MAX)
    { }

    TextureSamplerProperties myProperties;
    uint myGlobalDescriptorIndex;
  };
//---------------------------------------------------------------------------//
}