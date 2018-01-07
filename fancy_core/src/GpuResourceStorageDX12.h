#pragma once

#include "DX12Prerequisites.h"
#include "GpuResourceStorage.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class GpuResourceStorageDX12 : public GpuResourceStorage
  {
  public:
    GpuResourceStorageDX12() = default;
   
    Microsoft::WRL::ComPtr<ID3D12Resource> myResource;
  };
//---------------------------------------------------------------------------//
} }