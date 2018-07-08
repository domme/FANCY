#pragma once
#include "GpuResourceView.h"
#include "DescriptorDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct GpuResourceViewDataDX12
  {
    bool operator==(const GpuResourceViewDataDX12& anOther) const
    {
      return myType == anOther.myType &&
        myDescriptor.myHeapType == anOther.myDescriptor.myHeapType &&
        myDescriptor.myCpuHandle.ptr == anOther.myDescriptor.myCpuHandle.ptr &&
        myDescriptor.myGpuHandle.ptr == anOther.myDescriptor.myGpuHandle.ptr;
    }

    enum Type { 
      NONE = 0,
      CBV, SRV, UAV, DSV, RTV 
    };

    Type myType;
    DescriptorDX12 myDescriptor;
  };
//---------------------------------------------------------------------------//
}
