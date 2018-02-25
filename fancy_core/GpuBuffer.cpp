#include "GpuBuffer.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  GpuBuffer::GpuBuffer()
    : GpuResource(GpuResourceCategory::BUFFER)
    , myAlignment(0u)
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
}