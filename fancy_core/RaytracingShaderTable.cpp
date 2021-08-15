#include "fancy_core_precompile.h"
#include "RaytracingShaderTable.h"

#include "GpuBuffer.h"

using namespace Fancy;

RaytracingShaderTable::RaytracingShaderTable(const RaytracingShaderTableProperties& someProps, const SharedPtr<RaytracingPipelineState>& anRtPso)
  : myProperties(someProps)
  , myRtPso(anRtPso)
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

uint RaytracingShaderTable::AddShaderRecord(RaytracingShaderRecordType aType, uint aShaderIndexInRtPso)
{
  const RaytracingPipelineStateProperties& psoProps = myRtPso->myProperties;

  eastl::fixed_vector<uint8, 64> shaderRecordData;
  if (aType == RT_SHADER_RECORD_TYPE_HIT)
  {
    ASSERT(aShaderIndexInRtPso < (uint)psoProps.myHitGroups.size());
    GetShaderRecordDataInternal(aShaderIndexInRtPso, psoProps.myHitGroups[aShaderIndexInRtPso], shaderRecordData);
  }
  else
  {
    if (aType == RT_SHADER_RECORD_TYPE_RAYGEN)
    {
      ASSERT(aShaderIndexInRtPso < (uint)psoProps.myRaygenShaders.size());
      GetShaderRecordDataInternal(aShaderIndexInRtPso, psoProps.myRaygenShaders[aShaderIndexInRtPso], shaderRecordData);
    }
    else
    {
      ASSERT(aShaderIndexInRtPso < (uint)psoProps.myMissShaders.size());
      GetShaderRecordDataInternal(aShaderIndexInRtPso, psoProps.myMissShaders[aShaderIndexInRtPso], shaderRecordData);
    }
  }

  ASSERT(shaderRecordData.size() == myShaderRecordSizeBytes);
  ASSERT(myTypeRangeSizes[aType] + myShaderRecordSizeBytes <= myTypeRangeMaxSizes[aType]);

  uint8* dst = myMappedData + myTypeRangeOffsets[aType] + myTypeRangeSizes[aType];
  memcpy(dst, shaderRecordData.data(), myShaderRecordSizeBytes);

  const uint idx = myTypeRangeSizes[aType] / myShaderRecordSizeBytes;
  myTypeRangeSizes[aType] += myShaderRecordSizeBytes;

#if FANCY_HEAVY_DEBUG
  myShaders.push_back({ aType, aShaderIndexInRtPso });
#endif

  return idx;
}
