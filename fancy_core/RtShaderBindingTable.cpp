#include "fancy_core_precompile.h"
#include "RtShaderBindingTable.h"

#include "CommandList.h"
#include "GpuBuffer.h"

using namespace Fancy;

RtShaderBindingTable::RtShaderBindingTable(const RtShaderBindingTableProperties& someProps)
  : myProperties(someProps)
  , myMappedSbtData(nullptr)
  , myAlignedShaderRecordSizeBytes(0u)
  , myRecordTypeOffset{}
  , myNumUsedRecords{}
  , myMaxNumRecords{}
{
  const RenderPlatformCaps& caps = RenderCore::GetPlatformCaps();
  
  myAlignedShaderRecordSizeBytes = (uint) MathUtil::Align(caps.myRaytracingShaderIdentifierSizeBytes, caps.myRaytracingShaderRecordAlignment);
  ASSERT(myAlignedShaderRecordSizeBytes > 0 && myAlignedShaderRecordSizeBytes < caps.myRaytracingMaxShaderRecordSize);
  ASSERT(MathUtil::IsAligned(caps.myRaytracingShaderTableAddressAlignment, myAlignedShaderRecordSizeBytes));  // Make sure the table address alignment is a multiple of the shaderRecord size. The code below assumes we can express table-ranges as number of records

  uint tableAddressAlignmentInNumRecords = caps.myRaytracingShaderTableAddressAlignment / myAlignedShaderRecordSizeBytes;
  ASSERT(tableAddressAlignmentInNumRecords > 0);
  
  const uint numRecords[] = {
    myProperties.myNumRaygenShaderRecords,
    myProperties.myNumMissShaderRecords,
    myProperties.myNumHitShaderRecords
  };
  static_assert(ARRAY_LENGTH(numRecords) == RT_SHADER_IDENTIFIER_TYPE_NUM, "Invalid array size");

  uint overallNumRecords = 0;
  for (uint i = 0; i < RT_SHADER_IDENTIFIER_TYPE_NUM; ++i)
  {
    uint alignedNumRecords = (uint)MathUtil::Align(numRecords[i], tableAddressAlignmentInNumRecords);
    myMaxNumRecords[i] = alignedNumRecords;
    myRecordTypeOffset[i] = overallNumRecords;
    overallNumRecords += alignedNumRecords;
  }
  
  GpuBufferProperties bufferProps;
  bufferProps.myBindFlags = (uint) GpuBufferBindFlags::RAYTRACING_SHADER_BINDING_TABLE;
  bufferProps.myCpuAccess = CpuMemoryAccessType::CPU_WRITE;
  bufferProps.myElementSizeBytes = myAlignedShaderRecordSizeBytes;
  bufferProps.myNumElements = overallNumRecords;
  mySbtBuffer = RenderCore::CreateBuffer(bufferProps, "RT SBT Buffer");
  ASSERT(mySbtBuffer != nullptr);

  myMappedSbtData = (uint8*) mySbtBuffer->Map(GpuResourceMapMode::WRITE_UNSYNCHRONIZED);
  ASSERT(myMappedSbtData != nullptr);
}

uint RtShaderBindingTable::AddShaderRecord(const RtShaderIdentifier& aShaderIdentifier)
{
  RtShaderIdentifierType type = aShaderIdentifier.myType;

  ASSERT(myMaxNumRecords[type] > 0);
  ASSERT(myNumUsedRecords[type] < myMaxNumRecords[type]);
  ASSERT(aShaderIdentifier.myData.size() == myAlignedShaderRecordSizeBytes);

  const uint recordIdx = myRecordTypeOffset[type] + myNumUsedRecords[type];
  uint8* dst = myMappedSbtData + recordIdx * myAlignedShaderRecordSizeBytes;
  memcpy(dst, aShaderIdentifier.myData.data(), aShaderIdentifier.myData.size());
  ++myNumUsedRecords[type];

  return recordIdx;
}

RtShaderBindingTableRange RtShaderBindingTable::GetRange(RtShaderIdentifierType aType) const
{
  uint64 offsetBytes = myRecordTypeOffset[aType] * myAlignedShaderRecordSizeBytes;
  uint64 sizeBytes = myNumUsedRecords[aType] * myAlignedShaderRecordSizeBytes;
  return { mySbtBuffer.get(), offsetBytes, sizeBytes, myAlignedShaderRecordSizeBytes };
}
