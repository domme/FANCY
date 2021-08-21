#include "fancy_core_precompile.h"
#include "RaytracingShaderTable.h"

#include "GpuBuffer.h"

using namespace Fancy;

RaytracingShaderTable::RaytracingShaderTable(const RaytracingShaderTableProperties& someProps)
  : myProperties(someProps)
  , myMappedData(nullptr)
  , myShaderIdentifierSizeBytes(0u)
  , myAlignedShaderRecordSizeBytes(0u)
  , myTypeRangeOffsets{}
  , myTypeRangeSizes{}
  , myTypeRangeMaxSizes{}
{
  const RenderPlatformCaps& caps = RenderCore::GetPlatformCaps();
  myShaderIdentifierSizeBytes = caps.myRaytracingShaderIdentifierSizeBytes;

  myAlignedShaderRecordSizeBytes = (uint) MathUtil::Align(myShaderIdentifierSizeBytes + myProperties.myMaxShaderDescriptorDataSize, caps.myRaytracingShaderRecordAlignment);
  ASSERT(myAlignedShaderRecordSizeBytes > 0 && myAlignedShaderRecordSizeBytes < caps.myRaytracingMaxShaderRecordSize);

  myTypeRangeMaxSizes[RT_SHADER_RECORD_TYPE_RAYGEN] = (uint) MathUtil::Align(myProperties.myNumRaygenShaderRecords * myAlignedShaderRecordSizeBytes, caps.myRaytracingShaderTableAddressAlignment);
  myTypeRangeMaxSizes[RT_SHADER_RECORD_TYPE_MISS] = (uint) MathUtil::Align(myProperties.myNumMissShaderRecords * myAlignedShaderRecordSizeBytes, caps.myRaytracingShaderTableAddressAlignment);
  myTypeRangeMaxSizes[RT_SHADER_RECORD_TYPE_HIT] = (uint) MathUtil::Align(myProperties.myNumHitShaderRecords * myAlignedShaderRecordSizeBytes, caps.myRaytracingShaderTableAddressAlignment);

  myTypeRangeOffsets[RT_SHADER_RECORD_TYPE_RAYGEN] = 0ull;
  myTypeRangeOffsets[RT_SHADER_RECORD_TYPE_MISS] = myTypeRangeMaxSizes[RT_SHADER_RECORD_TYPE_RAYGEN];
  myTypeRangeOffsets[RT_SHADER_RECORD_TYPE_HIT] = myTypeRangeMaxSizes[RT_SHADER_RECORD_TYPE_RAYGEN] + myTypeRangeMaxSizes[RT_SHADER_RECORD_TYPE_MISS];

  GpuBufferProperties bufferProps;
  bufferProps.myBindFlags = (uint) GpuBufferBindFlags::RAYTRACING_SHADER_BINDING_TABLE;
  bufferProps.myCpuAccess = CpuMemoryAccessType::CPU_WRITE;
  bufferProps.myElementSizeBytes = myTypeRangeOffsets[RT_SHADER_RECORD_TYPE_HIT] + myTypeRangeMaxSizes[RT_SHADER_RECORD_TYPE_HIT];
  bufferProps.myNumElements = 1u;
  myBuffer = RenderCore::CreateBuffer(bufferProps, "RT SBT Buffer");
  ASSERT(myBuffer != nullptr);

  myMappedData = (uint8*) myBuffer->Map(GpuResourceMapMode::WRITE_UNSYNCHRONIZED);
  ASSERT(myMappedData != nullptr);
}

void RaytracingShaderTable::AddShaderRecord(const RaytracingShaderRecord& aShaderRecord)
{
  const RaytracingShaderRecordType type = aShaderRecord.myType;
  ASSERT(aShaderRecord.myData.size() <= myAlignedShaderRecordSizeBytes);
  ASSERT(myTypeRangeSizes[type] + myAlignedShaderRecordSizeBytes <= myTypeRangeMaxSizes[type]);

  uint8* dst = myMappedData + myTypeRangeOffsets[type] + myTypeRangeSizes[type];
  memcpy(dst, aShaderRecord.myData.data(), aShaderRecord.myData.size());
  
  myTypeRangeSizes[type] += myAlignedShaderRecordSizeBytes;
}

RaytracingShaderTableRange RaytracingShaderTable::GetRange(RaytracingShaderRecordType aType) const
{
  return { myBuffer.get(), myTypeRangeOffsets[aType], myTypeRangeSizes[aType], myAlignedShaderRecordSizeBytes };
}
