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
  , myTypeRangeCurrSizes{}
  , myTypeRangeMaxSizes{}
{
  const RenderPlatformCaps& caps = RenderCore::GetPlatformCaps();
  myShaderIdentifierSizeBytes = caps.myRaytracingShaderIdentifierSizeBytes;

  uint loclCbvSizeBytes = myProperties.myLocalCbufferOverallSize > 0 ? sizeof(uint64) : 0u;

  myAlignedShaderRecordSizeBytes = (uint) MathUtil::Align(myShaderIdentifierSizeBytes + loclCbvSizeBytes, caps.myRaytracingShaderRecordAlignment);
  ASSERT(myAlignedShaderRecordSizeBytes > 0 && myAlignedShaderRecordSizeBytes < caps.myRaytracingMaxShaderRecordSize);

  myTypeRangeMaxSizes[RT_SHADER_IDENTIFIER_TYPE_RAYGEN] = (uint) MathUtil::Align(myProperties.myNumRaygenShaderRecords * myAlignedShaderRecordSizeBytes, caps.myRaytracingShaderTableAddressAlignment);
  myTypeRangeMaxSizes[RT_SHADER_IDENTIFIER_TYPE_MISS] = (uint) MathUtil::Align(myProperties.myNumMissShaderRecords * myAlignedShaderRecordSizeBytes, caps.myRaytracingShaderTableAddressAlignment);
  myTypeRangeMaxSizes[RT_SHADER_IDENTIFIER_TYPE_HIT] = (uint) MathUtil::Align(myProperties.myNumHitShaderRecords * myAlignedShaderRecordSizeBytes, caps.myRaytracingShaderTableAddressAlignment);

  myTypeRangeOffsets[RT_SHADER_IDENTIFIER_TYPE_RAYGEN] = 0ull;
  myTypeRangeOffsets[RT_SHADER_IDENTIFIER_TYPE_MISS] = myTypeRangeMaxSizes[RT_SHADER_IDENTIFIER_TYPE_RAYGEN];
  myTypeRangeOffsets[RT_SHADER_IDENTIFIER_TYPE_HIT] = myTypeRangeMaxSizes[RT_SHADER_IDENTIFIER_TYPE_RAYGEN] + myTypeRangeMaxSizes[RT_SHADER_IDENTIFIER_TYPE_MISS];

  GpuBufferProperties bufferProps;
  bufferProps.myBindFlags = (uint) GpuBufferBindFlags::RAYTRACING_SHADER_BINDING_TABLE;
  bufferProps.myCpuAccess = CpuMemoryAccessType::CPU_WRITE;
  bufferProps.myElementSizeBytes = myTypeRangeOffsets[RT_SHADER_IDENTIFIER_TYPE_HIT] + myTypeRangeMaxSizes[RT_SHADER_IDENTIFIER_TYPE_HIT];
  bufferProps.myNumElements = 1u;
  myBuffer = RenderCore::CreateBuffer(bufferProps, "RT SBT Buffer");
  ASSERT(myBuffer != nullptr);

  myMappedData = (uint8*) myBuffer->Map(GpuResourceMapMode::WRITE_UNSYNCHRONIZED);
  ASSERT(myMappedData != nullptr);

  if (myProperties.myLocalCbufferOverallSize > 0)
  {
    GpuBufferProperties cbufferProps;
    cbufferProps.myCpuAccess = CpuMemoryAccessType::CPU_WRITE;
    cbufferProps.myBindFlags = (uint) GpuBufferBindFlags::CONSTANT_BUFFER;
    cbufferProps.myElementSizeBytes = myProperties.myLocalCbufferOverallSize;
    cbufferProps.myNumElements = 1u;
    myCbuffer.Create(cbufferProps, "RT Shader Table Local Cbuffer");
  }
}

void RaytracingShaderTable::AddShaderRecord(const RaytracingShaderIdentifier& aShaderIdentifier, void* aCbufferData, uint64 aCbufferDataSize)
{
  ASSERT(!aCbufferData || myCbuffer.GetBuffer() != nullptr);

  uint64 cbufferWriteOffset = 0ull;
  if (aCbufferData)
  {
    bool cbufferWriteSuccess = myCbuffer.AllocateAndWrite(aCbufferData, aCbufferDataSize, cbufferWriteOffset);
    ASSERT(cbufferWriteSuccess);
  }
  
  const uint writeSizeBytes = (uint) aShaderIdentifier.myData.size() + (aCbufferData ? sizeof(uint64) : 0u);
  ASSERT(writeSizeBytes <= myAlignedShaderRecordSizeBytes);
  
  const RaytracingShaderIdentifierType type = aShaderIdentifier.myType;
  ASSERT((uint) aShaderIdentifier.myData.size() <= myAlignedShaderRecordSizeBytes);
  ASSERT(myTypeRangeCurrSizes[type] + myAlignedShaderRecordSizeBytes <= myTypeRangeMaxSizes[type]);
  
  uint8* dst = myMappedData + myTypeRangeOffsets[type] + myTypeRangeCurrSizes[type];
  memcpy(dst, aShaderIdentifier.myData.data(), aShaderIdentifier.myData.size());
  const uint numBytesWritten = (uint) aShaderIdentifier.myData.size();

  if (aCbufferData)
  {
    ASSERT((numBytesWritten < myAlignedShaderRecordSizeBytes) && (myAlignedShaderRecordSizeBytes - numBytesWritten >= sizeof(uint64)));
    const uint64 cbv = myCbuffer.GetBuffer()->GetDeviceAddress() + cbufferWriteOffset;
    memcpy(dst + numBytesWritten, &cbv, sizeof(cbv));
  }
  
  myTypeRangeCurrSizes[type] += myAlignedShaderRecordSizeBytes;
}

void RaytracingShaderTable::SetShaderRecordData_WaitForGPU(RaytracingShaderIdentifierType aType, uint aShaderRecordIndex, void* aCbufferData, uint64 aCbufferDataSize)
{

}

RaytracingShaderTableRange RaytracingShaderTable::GetRange(RaytracingShaderIdentifierType aType) const
{
  return { myCbuffer.GetBuffer(), myBuffer.get(), myTypeRangeOffsets[aType], myTypeRangeCurrSizes[aType], myAlignedShaderRecordSizeBytes };
}
