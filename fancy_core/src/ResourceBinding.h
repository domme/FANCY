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
  typedef void (BindingFunction)(RenderContext*, MaterialPassInstance*);
//---------------------------------------------------------------------------//
  void Register();
  void RegisterForResourceInterface(uint64 anInterfaceHash, BindingFunction* aBindingFunction);
  BindingFunction* GetFromResourceInterface(uint64 anInterfaceHash);

  // Binding-functions. Implementations need to be synced with shader/inc_RootSignatures.hlsl
  void BindResources_ForwardColorPass(RenderContext* aContext, MaterialPassInstance* aMaterial);
//---------------------------------------------------------------------------//
} } }  // end of namespace Fancy::Rendering::ResourceBinding
