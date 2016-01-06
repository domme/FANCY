#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  /// Platform-independent abstraction of the "RootSignature" concept in DX12. Not used in GL4 currently
  class GpuResourceSignature
  {
    // TODO: Implement with data-layout resembling the native DX12-RootSignature Desc. 
    // Use a hash to quickly lookup the corresponding native RootSignature object
  };
//---------------------------------------------------------------------------//
} }


