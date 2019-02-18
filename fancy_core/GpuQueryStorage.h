#pragma once

#include "FancyCoreDefines.h"
#include "Ptr.h"

namespace Fancy
{
  class GpuQueryHeap;
  class GpuBuffer;

  struct GpuQueryStorage
  {
    SharedPtr<GpuBuffer> myReadbackBuffer;
    SharedPtr<GpuQueryHeap> myQueryHeap;
  };
}

