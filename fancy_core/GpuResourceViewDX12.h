#pragma once
#include "GpuResourceView.h"
#include "DescriptorDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class GpuResourceViewDX12 final : public GpuResourceView
  {
  public:
    DescriptorDX12 myDescriptor;
  };
//---------------------------------------------------------------------------//
}
