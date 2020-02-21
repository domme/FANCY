#pragma once

#include "TextureSampler.h"
#include "DescriptorDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class TextureSamplerDX12 : public TextureSampler
  {
  public:
    TextureSamplerDX12(const TextureSamplerProperties& someProperties);
    ~TextureSamplerDX12() override;

    const DescriptorDX12& GetDescriptor() const { return myDescriptor; }

  protected:
    DescriptorDX12 myDescriptor;
  };
//---------------------------------------------------------------------------//
}

