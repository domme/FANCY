#pragma once

#include "RendererPrerequisites.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class RenderContext;
  class MaterialPassInstance;
//---------------------------------------------------------------------------//
  class ResourceBinding
  {
  public:
    ResourceBinding() {}
    virtual ~ResourceBinding() {}

    virtual void ApplyResources(RenderContext* aContext, MaterialPassInstance* aMaterial) = 0;
  };
//---------------------------------------------------------------------------//
  class ResourceBindingForwardColor : public ResourceBinding  //< Binds RootSignature RS_FORWARD_COLORPASS
  {
  public:
    void ApplyResources(RenderContext* aContext, MaterialPassInstance* aMaterial) override;
  };
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering
