#include "fancy_core_precompile.h"
#include "RaytracingShaderTable.h"

#include "GpuBuffer.h"

using namespace Fancy;

RaytracingShaderTable::RaytracingShaderTable(const RaytracingShaderTableProperties& someProps)
  : myProperties(someProps)
  , myMappedData(nullptr)
  , myShaderRecordSizeBytes(0u)
  , myTypeRangeOffsets{}
  , myTypeRangeSizes{}
  , myTypeRangeMaxSizes{}
{
  myShaderRecordSizeBytes = RenderCore::GetPlatformCaps().myRaytracingShaderIdentifierSizeBytes;
  ASSERT(myShaderRecordSizeBytes > 0);

  myTypeRangeOffsets[RT_SHADER_RECORD_TYPE_RAYGEN] = 0ull;
  myTypeRangeOffsets[RT_SHADER_RECORD_TYPE_MISS] = myProperties.myNumRaygenShaderRecords * myShaderRecordSizeBytes;
  myTypeRangeOffsets[RT_SHADER_RECORD_TYPE_HIT] = (myProperties.myNumRaygenShaderRecords + myProperties.myNumMissShaderRecords) * myShaderRecordSizeBytes;

  myTypeRangeMaxSizes[RT_SHADER_RECORD_TYPE_RAYGEN] = myProperties.myNumRaygenShaderRecords * myShaderRecordSizeBytes;
  myTypeRangeMaxSizes[RT_SHADER_RECORD_TYPE_MISS] = myProperties.myNumMissShaderRecords * myShaderRecordSizeBytes;
  myTypeRangeMaxSizes[RT_SHADER_RECORD_TYPE_HIT] = myProperties.myNumHitShaderRecords * myShaderRecordSizeBytes;

  GpuBufferProperties bufferProps;
  bufferProps.myBindFlags = (uint) GpuBufferBindFlags::RAYTRACING_SHADER_BINDING_TABLE;
  bufferProps.myCpuAccess = CpuMemoryAccessType::CPU_WRITE;
  bufferProps.myElementSizeBytes = myShaderRecordSizeBytes;
  bufferProps.myNumElements = myProperties.myNumRaygenShaderRecords + myProperties.myNumMissShaderRecords + myProperties.myNumHitShaderRecords;
  myBuffer = RenderCore::CreateBuffer(bufferProps, "RT SBT Buffer");
  ASSERT(myBuffer != nullptr);

  myMappedData = (uint8*) myBuffer->Map(GpuResourceMapMode::WRITE_UNSYNCHRONIZED);
  ASSERT(myMappedData != nullptr);
}

uint RaytracingShaderTable::AddShaderRecord(const RaytracingShaderRecord& aShaderRecord)
{
  const RaytracingShaderRecordType type = aShaderRecord.myType;
  ASSERT(aShaderRecord.myData.size() == myShaderRecordSizeBytes);
  ASSERT(myTypeRangeSizes[type] + myShaderRecordSizeBytes <= myTypeRangeMaxSizes[type]);

  uint8* dst = myMappedData + myTypeRangeOffsets[type] + myTypeRangeSizes[type];
  memcpy(dst, aShaderRecord.myData.data(), myShaderRecordSizeBytes);

  const uint idx = myTypeRangeSizes[type] / myShaderRecordSizeBytes;
  myTypeRangeSizes[type] += myShaderRecordSizeBytes;

  return idx;
}
