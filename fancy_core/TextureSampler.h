#pragma once

#include "TextureSamplerProperties.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class TextureSampler
  {
  public:
    virtual ~TextureSampler() = default;

    const TextureSamplerProperties GetProperties() const { return myProperties; }

  protected:
    TextureSampler(const TextureSamplerProperties& someProperties)
      : myProperties(someProperties)
    { }

    TextureSamplerProperties myProperties;
  };
//---------------------------------------------------------------------------//
}