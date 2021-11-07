#include "fancy_core_precompile.h"
#include "RtAccelerationStructure.h"
#include "CommandList.h"
#include "GpuBuffer.h"

namespace Fancy
{
  uint64 RtBufferData::GetGpuBufferAddress(CommandList* aCommandList, uint anAlingment) const
  {
    if (myType == RT_BUFFER_DATA_TYPE_NONE)
      return 0;

    if (myType == RT_BUFFER_DATA_TYPE_GPU_BUFFER)
    {
      uint64 startAddress = myBuffer.myBuffer->GetDeviceAddress() + myBuffer.myOffsetBytes;
      ASSERT(MathUtil::IsAligned(startAddress, anAlingment));
      return startAddress;
    }

    uint64 offset;
    const GpuBuffer* buffer = aCommandList->GetBuffer(offset, GpuBufferUsage::STAGING_UPLOAD, myCpuData.myData, myCpuData.myDataSize, anAlingment);
    ASSERT(buffer != nullptr);
    return buffer->GetDeviceAddress() + offset;
  }

  RtAccelerationStructure::RtAccelerationStructure(RtAccelerationStructureType aType, const char* aName)
    : myType(aType)
    , myName(aName ? aName : "")
  {
  }
}
