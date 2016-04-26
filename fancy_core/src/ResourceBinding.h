#pragma once

#include "RendererPrerequisites.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class RenderContext;
  class MaterialPassInstance;
//---------------------------------------------------------------------------//
} }

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct ResourceBindingDataSource
  {
    const MaterialPassInstance* myMaterial;
    const GpuBuffer** myConstantBuffers;
    uint32 myNumConstantBuffers;

    // TODO: Add support for primitive data ("RootConstants" in DX12)
  };
//---------------------------------------------------------------------------//
} }

namespace Fancy { namespace Rendering { namespace ResourceBinding {
//---------------------------------------------------------------------------//
  // Binding-functions. Implementations need to be synced with shader/inc_RootSignatures.hlsl
  void BindResources_ForwardColorPass(RenderContext* aRenderContext, const ResourceBindingDataSource& aDataSource);
//---------------------------------------------------------------------------//
} } }  // end of namespace Fancy::Rendering::ResourceBinding
