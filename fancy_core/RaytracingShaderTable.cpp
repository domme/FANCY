#include "fancy_core_precompile.h"
#include "RaytracingShaderTable.h"

#include "CommandList.h"
#include "GpuBuffer.h"

using namespace Fancy;

RaytracingShaderTable::RaytracingShaderTable(const RaytracingShaderTableProperties& someProps)
  : myProperties(someProps)
  , myMappedSbtData(nullptr)
  , myAlignedShaderRecordSizeBytes(0u)
  , mySbtOffset(0u)
{
  const RenderPlatformCaps& caps = RenderCore::GetPlatformCaps();
  
  myAlignedShaderRecordSizeBytes = (uint) MathUtil::Align(caps.myRaytracingShaderIdentifierSizeBytes, caps.myRaytracingShaderRecordAlignment);
  ASSERT(myAlignedShaderRecordSizeBytes > 0 && myAlignedShaderRecordSizeBytes < caps.myRaytracingMaxShaderRecordSize);

  uint64 sbtMaxSize = MathUtil::Align(myProperties.myMaxNumRecords * myAlignedShaderRecordSizeBytes, caps.myRaytracingShaderTableAddressAlignment);

  GpuBufferProperties bufferProps;
  bufferProps.myBindFlags = (uint) GpuBufferBindFlags::RAYTRACING_SHADER_BINDING_TABLE;
  bufferProps.myCpuAccess = CpuMemoryAccessType::CPU_WRITE;
  bufferProps.myElementSizeBytes = sbtMaxSize;
  bufferProps.myNumElements = 1u;
  mySbtBuffer = RenderCore::CreateBuffer(bufferProps, "RT SBT Buffer");
  ASSERT(mySbtBuffer != nullptr);

  myMappedSbtData = (uint8*) mySbtBuffer->Map(GpuResourceMapMode::WRITE_UNSYNCHRONIZED);
  ASSERT(myMappedSbtData != nullptr);
}

uint RaytracingShaderTable::AddShaderRecord(const RaytracingShaderIdentifier& aShaderIdentifier)
{
  ASSERT(aShaderIdentifier.myType == myProperties.myType);

  // Write the shader record (shader identifier data + cbuffer address for use as a root-CBV)
  const uint shaderRecordWriteSize = (uint)aShaderIdentifier.myData.size();
  ASSERT(shaderRecordWriteSize <= myAlignedShaderRecordSizeBytes);
  ASSERT((uint)aShaderIdentifier.myData.size() <= myAlignedShaderRecordSizeBytes);
  ASSERT(mySbtOffset + myAlignedShaderRecordSizeBytes <= mySbtBuffer->GetByteSize());
  
  uint8* dst = myMappedSbtData + mySbtOffset;
  memcpy(dst, aShaderIdentifier.myData.data(), aShaderIdentifier.myData.size());
  const uint numBytesWritten = (uint) aShaderIdentifier.myData.size();

  const uint idx = uint(mySbtOffset / (uint64) myAlignedShaderRecordSizeBytes);
  mySbtOffset += myAlignedShaderRecordSizeBytes;
  return idx;
}

RaytracingShaderTableRange RaytracingShaderTable::GetRange() const
{
  return { mySbtBuffer.get(), 0ull, mySbtOffset, myAlignedShaderRecordSizeBytes };
}
