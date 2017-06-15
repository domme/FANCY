#pragma once

#include "RendererPrerequisites.h"
#include "CommandListType.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class GpuResource;
//---------------------------------------------------------------------------//
  enum class ResourceBindingType
  {
    SIMPLE = 0,
    READ_WRITE,
    RENDER_TARGET,
    DEPTH_STENCIL_TARGET,
    CONSTANT_BUFFER
  };
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
    virtual void BindResource(const GpuResource* aResource, ResourceBindingType aBindingType, uint32 aRegisterIndex) const = 0;
    
    // Descriptor tables:
    virtual void BindResourceSet(const GpuResource** someResources, ResourceBindingType* someBindingTypes, uint32 aResourceCount, uint32 aRegisterIndex) = 0;

    virtual void Reset() = 0;
    virtual uint64 ExecuteAndReset(bool aWaitForCompletion = false) = 0;

  protected:
    CommandListType myCommandListType;
  };
//---------------------------------------------------------------------------//
} }
