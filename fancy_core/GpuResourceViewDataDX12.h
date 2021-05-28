#pragma once

#include "DescriptorDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
//---------------------------------------------------------------------------//
  struct GpuResourceViewDataDX12
  {
    DescriptorDX12 myDescriptor;  // Shader-visible descriptor for SRV, UAV, CBV. Non-shader-visible for all others (DSV, RTV)
  };
//---------------------------------------------------------------------------//
}

#endif