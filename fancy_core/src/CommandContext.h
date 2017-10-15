#pragma once

#include "RendererPrerequisites.h"
#include "CommandListType.h"
#include "Descriptor.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class Descriptor;
  class GpuResource;  
//---------------------------------------------------------------------------//
  class DLLEXPORT CommandContext
  {
  public:
    CommandContext(CommandListType aType);
    virtual ~CommandContext() {}

    CommandListType GetType() const { return myCommandListType; }

    virtual void ClearRenderTarget(Texture* aTexture, const float* aColor) = 0;
    virtual void ClearDepthStencilTarget(Texture* aTexture, float aDepthClear, uint8 aStencilClear, uint32 someClearFlags = (uint32)DepthStencilClearFlags::CLEAR_ALL) = 0;

    // Root arguments:
    virtual void BindResource(const GpuResource* aResource, DescriptorType aBindingType, uint32 aRegisterIndex) const = 0;
    
    // Descriptor tables:
    virtual void BindDescriptorSet(const Descriptor** someDescriptors, uint32 aResourceCount, uint32 aRegisterIndex) = 0;

    virtual void Reset() = 0;
    virtual uint64 ExecuteAndReset(bool aWaitForCompletion = false) = 0;

  protected:
    CommandListType myCommandListType;
  };
//---------------------------------------------------------------------------//
} }
