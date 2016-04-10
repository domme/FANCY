#pragma once

#include "RendererPrerequisites.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class RenderContext;
  class MaterialPassInstance;
//---------------------------------------------------------------------------//
} }

namespace Fancy { namespace Rendering { namespace ResourceBinding {
//---------------------------------------------------------------------------//
  void BindResources_ForwardColorPass(RenderContext* aContext, MaterialPassInstance* aMaterial);
//---------------------------------------------------------------------------//
} } }  // end of namespace Fancy::Rendering::ResourceBinding
