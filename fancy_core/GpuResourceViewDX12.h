#pragma once
#include "GpuResourceView.h"
#include "DescriptorDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct GpuResourceViewDataDX12
  {
    enum Type { 
      NONE = 0,
      CBV, SRV, UAV, DSV, RTV 
    };

    Type myType;
    DescriptorDX12 myDescriptor;
  };
//---------------------------------------------------------------------------//
}
