#pragma once
#include "GpuResourceView.h"
#include "DescriptorDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct GpuResourceViewDataDX12
  {
    enum Type { 
      NONE = 0,
      CBV, SRV, UAV, DSV, RTV 
    };

    bool operator==(const GpuResourceViewDataDX12& anOther) const
    {
      return myType == anOther.myType &&
        myDescriptor.myHeapType == anOther.myDescriptor.myHeapType &&
        myDescriptor.myCpuHandle.ptr == anOther.myDescriptor.myCpuHandle.ptr &&
        myDescriptor.myGpuHandle.ptr == anOther.myDescriptor.myGpuHandle.ptr;
    }

    Type myType;
    DescriptorDX12 myDescriptor;
  };
//---------------------------------------------------------------------------//
}
