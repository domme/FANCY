#include "GpuBuffer.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  GpuBuffer::GpuBuffer()
    : GpuResource(GpuResourceCategory::BUFFER)
    , myAlignment(0u)
  {
  }
//---------------------------------------------------------------------------//
}