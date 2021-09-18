#include "fancy_core_precompile.h"
#include "RaytracingShaderTable.h"

#include "CommandList.h"
#include "GpuBuffer.h"

using namespace Fancy;

RaytracingShaderTable::RaytracingShaderTable(const RaytracingShaderTableProperties& someProps)
  : myProperties(someProps)
  , myMappedSbtData(nullptr)
  , myShaderIdentifierSizeBytes(0u)
  , myAlignedShaderRecordSizeBytes(0u)
  , mySbtOffset(0u)
  , myCbufferOffset(0u)
{
  const RenderPlatformCaps& caps = RenderCore::GetPlatformCaps();
  myShaderIdentifierSizeBytes = caps.myRaytracingShaderIdentifierSizeBytes;

  uint loclCbvSizeBytes = myProperties.myMaxCbufferSize > 0 ? sizeof(uint64) : 0u;

  myAlignedShaderRecordSizeBytes = (uint) MathUtil::Align(myShaderIdentifierSizeBytes + loclCbvSizeBytes, caps.myRaytracingShaderRecordAlignment);
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
  
  if (myProperties.myMaxCbufferSize > 0)
  {
    GpuBufferProperties cbufferProps;
    cbufferProps.myCpuAccess = CpuMemoryAccessType::CPU_WRITE;
    cbufferProps.myBindFlags = (uint) GpuBufferBindFlags::CONSTANT_BUFFER;
    cbufferProps.myElementSizeBytes = myProperties.myMaxCbufferSize;
    cbufferProps.myNumElements = 1u;
    myCbuffer = RenderCore::CreateBuffer(cbufferProps, "RT Shader Table Local Cbuffer");
  }
}

uint RaytracingShaderTable::AddShaderRecord(const RaytracingShaderIdentifier& aShaderIdentifier, uint64 aCbufferDataSize /*= 0u*/, void* aCbufferData /*= nullptr*/)
{
  ASSERT(aShaderIdentifier.myType == myProperties.myType);
  ASSERT(!aCbufferData || myCbuffer != nullptr);
  ASSERT(!aCbufferData || aCbufferDataSize > 0);
  ASSERT(aCbufferDataSize == 0 || myCbuffer != nullptr);

  // Write cbuffer data and store range for this record
  uint64 cbufferOffset = myCbufferOffset;
  if (aCbufferDataSize > 0)
  {
    ASSERT(cbufferOffset + aCbufferDataSize <= myCbuffer->GetByteSize());

    myCbufferRangePerRecord.push_back({ myCbufferOffset, aCbufferDataSize });
    myCbufferOffset += aCbufferDataSize;
  }
  else
  {
    myCbufferRangePerRecord.push_back({ UINT64_MAX, UINT64_MAX });
  }
  
  if (aCbufferData)
  {
    void* cbufferMappedData = myCbuffer->Map(GpuResourceMapMode::WRITE_UNSYNCHRONIZED, cbufferOffset, aCbufferDataSize);
    memcpy(cbufferMappedData, aCbufferData, aCbufferDataSize);
    myCbuffer->Unmap(GpuResourceMapMode::WRITE_UNSYNCHRONIZED, cbufferOffset, aCbufferDataSize);
  }

  // Write the shader record (shader identifier data + cbuffer address for use as a root-CBV)
  const uint shaderRecordWriteSize = (uint) aShaderIdentifier.myData.size() + (aCbufferDataSize ? sizeof(uint64) : 0u);
  ASSERT(shaderRecordWriteSize <= myAlignedShaderRecordSizeBytes);
  ASSERT((uint)aShaderIdentifier.myData.size() <= myAlignedShaderRecordSizeBytes);
  ASSERT(mySbtOffset + myAlignedShaderRecordSizeBytes <= mySbtBuffer->GetByteSize());
  
  uint8* dst = myMappedSbtData + mySbtOffset;
  memcpy(dst, aShaderIdentifier.myData.data(), aShaderIdentifier.myData.size());
  const uint numBytesWritten = (uint) aShaderIdentifier.myData.size();
  
  if (aCbufferDataSize > 0)
  {
    ASSERT((numBytesWritten < myAlignedShaderRecordSizeBytes) && (myAlignedShaderRecordSizeBytes - numBytesWritten >= sizeof(uint64)));
    const uint64 cbv = myCbuffer->GetDeviceAddress() + cbufferOffset;
    memcpy(dst + numBytesWritten, &cbv, sizeof(cbv));
  }
  
  mySbtOffset += myAlignedShaderRecordSizeBytes;

  return (uint) myCbufferRangePerRecord.size();
}

void RaytracingShaderTable::UpdateCbufferData(CommandList* aCommandList, uint aRecordIdx, uint64 aSize, void* aData)
{
  ASSERT(aData != nullptr);
  ASSERT(aRecordIdx < (uint)myCbufferRangePerRecord.size());

  const eastl::pair<uint64, uint64>& cbufferRange = myCbufferRangePerRecord[aRecordIdx];
  ASSERT(cbufferRange.first != UINT64_MAX && cbufferRange.second != UINT64_MAX);
  ASSERT(cbufferRange.second == aSize);

  uint64 srcBufferOffset = 0ull;
  const GpuBuffer* srcBuffer = aCommandList->GetBuffer(srcBufferOffset, GpuBufferUsage::STAGING_UPLOAD, aData, aSize);

  aCommandList->CopyBuffer(myCbuffer.get(), cbufferRange.first, srcBuffer, srcBufferOffset, aSize);
}

RaytracingShaderTableRange RaytracingShaderTable::GetRange() const
{
  return { myCbuffer.get(), mySbtBuffer.get(), 0ull, mySbtOffset, myAlignedShaderRecordSizeBytes };
}
