#include "GpuBuffer.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  GpuBuffer::GpuBuffer()
    : myAlignment(0u)
  {
  }
//---------------------------------------------------------------------------//
  GpuBuffer::~GpuBuffer()
  {
  }
//---------------------------------------------------------------------------//
  bool GpuBuffer::operator==(const GpuBufferDesc& aDesc) const
  {
    return GetDescription() == aDesc;
  }
//---------------------------------------------------------------------------//
  GpuBufferDesc GpuBuffer::GetDescription() const
  {
    GpuBufferDesc desc;
    desc.myInternalRefIndex = myParameters.myInternalRefIndex;
    return desc;
  }
//---------------------------------------------------------------------------//
} }