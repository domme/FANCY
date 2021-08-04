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

  GpuBufferProperties bufferProps;
  bufferProps.myBindFlags = (uint) GpuBufferBindFlags::RAYTRACING_SHADER_BINDING_TABLE;
  bufferProps.myCpuAccess = CpuMemoryAccessType::CPU_WRITE;
  bufferProps.myElementSizeBytes = myShaderRecordSizeBytes;
  bufferProps.myNumElements = myMaxNumShaderRecords;
  myBuffer = RenderCore::CreateBuffer(bufferProps, "RT SBT Buffer");
  ASSERT(myBuffer != nullptr);

  myMappedData = myBuffer->Map(GpuResourceMapMode::WRITE_UNSYNCHRONIZED);
  ASSERT(myMappedData != nullptr);
}
