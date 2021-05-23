#pragma once

#include "TextureSampler.h"
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
    const DescriptorDX12& GetBindlessShaderVisibleDescriptor() const { return myBindlessShaderVisibleDescriptor; }

  protected:
    DescriptorDX12 myDescriptor;
    DescriptorDX12 myBindlessShaderVisibleDescriptor;
  };
//---------------------------------------------------------------------------//
}

#endif