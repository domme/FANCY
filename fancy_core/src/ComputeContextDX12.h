#pragma once

#include "CommandContext.h"

namespace Fancy{
namespace Rendering{
class Descriptor;
}
}

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class ComputeContextDX12 : public CommandContext
  {
  public:
    // Root arguments:
    void SetReadTexture(const Texture* aTexture, uint32 aRegisterIndex) const;
    void SetWriteTexture(const Texture* aTexture, uint32 aRegisterIndex) const;
    void SetReadBuffer(const GpuBuffer* aBuffer, uint32 aRegisterIndex) const;
    void SetConstantBuffer(const GpuBuffer* aConstantBuffer, uint32 aRegisterIndex) const;

    // Descriptor tables:
    void SetMultipleResources(const Descriptor* someResources, uint32 aResourceCount, uint32 aDescriptorTypeMask, uint32 aRegisterIndex);
  };
//---------------------------------------------------------------------------//
} } }
