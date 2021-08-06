#include "fancy_core_precompile.h"
#include "RaytracingShaderTableDX12.h"

#include "GpuBuffer.h"
#include "RaytracingPipelineStateDX12.h"
#include "RenderCore_PlatformDX12.h"

using namespace Fancy;

RaytracingShaderTableDX12::RaytracingShaderTableDX12(RaytracingShaderTableType aType, uint aMaxNumShaderRecords, const SharedPtr<RaytracingPipelineState>& anRtPso)
  : RaytracingShaderTable(aType, aMaxNumShaderRecords, anRtPso)
{
  auto device = RenderCore::GetPlatformDX12()->GetDevice();
  RaytracingPipelineStateDX12* rtPsoDx12 = (RaytracingPipelineStateDX12*)anRtPso.get();
  ASSERT_HRESULT(rtPsoDx12->myStateObject.As(&myRtPsoProperties));
}

void RaytracingShaderTableDX12::AddShaderRecordInternal(uint /*aShaderIndexInRtPso*/, const RaytracingPipelineStateProperties::HitGroup& aHitGroup)
{
  WriteShaderIdentifier(aHitGroup.myName.c_str());
}

void RaytracingShaderTableDX12::AddShaderRecordInternal(uint /*aShaderIndexInRtPso*/, const RaytracingPipelineStateProperties::ShaderEntry& aShaderEntry)
{
  WriteShaderIdentifier(aShaderEntry.myUniqueMainFunctionName.c_str());
}

void RaytracingShaderTableDX12::WriteShaderIdentifier(const wchar_t* aUniqueName)
{
  void* shaderIdentifier = myRtPsoProperties->GetShaderIdentifier(aUniqueName);
  ASSERT(shaderIdentifier);
  ASSERT(myWriteOffset + myShaderRecordSizeBytes <= myBuffer->GetByteSize());

  uint8* dstPtr = myMappedData + myWriteOffset;
  memcpy(dstPtr, shaderIdentifier, myShaderRecordSizeBytes);
  myWriteOffset += myShaderRecordSizeBytes;
}
