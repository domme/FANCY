#pragma once

#include "RendererPrerequisites.h"
#include "CommandListType.h"

namespace Fancy { namespace Rendering {
  class Descriptor;

  //---------------------------------------------------------------------------//
  class CommandContext
  {
  public:
    CommandContext(CommandListType aType);
    virtual ~CommandContext() {}

    CommandListType GetType() const { return myCommandListType; }

    virtual void ClearRenderTarget(Texture* aTexture, const float* aColor) = 0;
    virtual void ClearDepthStencilTarget(Texture* aTexture, float aDepthClear, uint8 aStencilClear, uint32 someClearFlags = (uint32)DepthStencilClearFlags::CLEAR_ALL) const = 0;

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
