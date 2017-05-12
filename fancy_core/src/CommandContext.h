#pragma once

#include "RendererPrerequisites.h"
#include "CommandListType.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class CommandContext
  {
  public:
    CommandContext(CommandListType aType);
    virtual ~CommandContext() {}

    CommandListType GetType() const { return myCommandListType; }

    // Root arguments:
    virtual void SetReadTexture(const Texture* aTexture, uint32 aRegisterIndex) const = 0;
    virtual void SetWriteTexture(const Texture* aTexture, uint32 aRegisterIndex) const = 0;
    virtual void SetReadBuffer(const GpuBuffer* aBuffer, uint32 aRegisterIndex) const = 0;
    virtual void SetConstantBuffer(const GpuBuffer* aConstantBuffer, uint32 aRegisterIndex) const = 0;

    // Descriptor tables:
    virtual void SetMultipleResources(const Descriptor* someResources, uint32 aResourceCount, uint32 aRegisterIndex) = 0;

    virtual void Reset() = 0;
    virtual uint64 ExecuteAndReset(bool aWaitForCompletion = false) = 0;

  protected:
    CommandListType myCommandListType;
  };
//---------------------------------------------------------------------------//
} }
