#pragma once

#include "DescriptorDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
//---------------------------------------------------------------------------//
  struct GpuResourceViewDataDX12
  {
    DescriptorDX12 myDescriptor;
    DescriptorDX12 myBindlessShaderVisibleDescriptor;
  };
//---------------------------------------------------------------------------//
}

#endif