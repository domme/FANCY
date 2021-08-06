#include "fancy_core_precompile.h"
#include "RaytracingShaderTable.h"

#include "GpuBuffer.h"

using namespace Fancy;

RaytracingShaderTable::RaytracingShaderTable(RaytracingShaderTableType aType, uint aMaxNumShaderRecords, const SharedPtr<RaytracingPipelineState>& anRtPso)
  : myType(aType)
  , myMaxNumShaderRecords(aMaxNumShaderRecords)
  , myShaderRecordSizeBytes(0u)
  , myRtPso(anRtPso)
  , myMappedData(nullptr)
  , myWriteOffset(0u)
{
  myShaderRecordSizeBytes = RenderCore::GetPlatformCaps().myRaytracingShaderIdentifierSizeBytes;
  ASSERT(myShaderRecordSizeBytes > 0);

  GpuBufferProperties bufferProps;
  bufferProps.myBindFlags = (uint) GpuBufferBindFlags::RAYTRACING_SHADER_BINDING_TABLE;
  bufferProps.myCpuAccess = CpuMemoryAccessType::CPU_WRITE;
  bufferProps.myElementSizeBytes = myShaderRecordSizeBytes;
  bufferProps.myNumElements = myMaxNumShaderRecords;
  myBuffer = RenderCore::CreateBuffer(bufferProps, "RT SBT Buffer");
  ASSERT(myBuffer != nullptr);

  myMappedData = (uint8*) myBuffer->Map(GpuResourceMapMode::WRITE_UNSYNCHRONIZED);
  ASSERT(myMappedData != nullptr);
}

uint RaytracingShaderTable::AddShaderRecord(uint aShaderIndexInRtPso)
{
  const RaytracingPipelineStateProperties& psoProps = myRtPso->myProperties;

  if (myType == RT_SHADER_TABLE_TYPE_HIT)
  {
    ASSERT(aShaderIndexInRtPso < (uint)psoProps.myHitGroups.size());
    AddShaderRecordInternal(aShaderIndexInRtPso, psoProps.myHitGroups[aShaderIndexInRtPso]);
  }
  else
  {
    if (myType == RT_SHADER_TABLE_TYPE_RAYGEN)
    {
      ASSERT(aShaderIndexInRtPso < (uint)psoProps.myRaygenShaders.size());
      AddShaderRecordInternal(aShaderIndexInRtPso, psoProps.myRaygenShaders[aShaderIndexInRtPso]);
    }
    else
    {
      ASSERT(aShaderIndexInRtPso < (uint)psoProps.myMissShaders.size());
      AddShaderRecordInternal(aShaderIndexInRtPso, psoProps.myMissShaders[aShaderIndexInRtPso]);
    }
  }

#if FANCY_HEAVY_DEBUG
  myShaders.push_back(aShaderIndexInRtPso);
#endif

  return (myWriteOffset - myShaderRecordSizeBytes) / myShaderRecordSizeBytes;
}
