#pragma once

#include "Rendering/TextureSampler.h"
#include "DescriptorDX12.h"

#if FANCY_ENABLE_DX12

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

#endif