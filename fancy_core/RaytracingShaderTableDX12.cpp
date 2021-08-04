#include "fancy_core_precompile.h"
#include "RaytracingShaderTableDX12.h"

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

uint RaytracingShaderTableDX12::AddShaderRecord(uint aShaderIndexInRtPso)
{
  const RaytracingPipelineStateProperties& psoProps = myRtPso->myProperties;

  const wchar_t* shaderName = nullptr;
  if (myType == RT_SHADER_TABLE_TYPE_RAYGEN)
  {
    ASSERT(aShaderIndexInRtPso < (uint) psoProps.myRaygenShaders.size());
    shaderName = psoProps.myRaygenShaders[aShaderIndexInRtPso].myUniqueMainFunctionName.c_str();
  }
  else if (myType == RT_SHADER_TABLE_TYPE_MISS)
  {
    ASSERT(aShaderIndexInRtPso < (uint)psoProps.myMissShaders.size());
    shaderName = psoProps.myMissShaders[aShaderIndexInRtPso].myUniqueMainFunctionName.c_str();
  }
  else
  {
    ASSERT(aShaderIndexInRtPso < (uint)psoProps.myHitGroups.size());
    shaderName = psoProps.myHitGroups[aShaderIndexInRtPso].myName.c_str();
  }

  void* shaderIdentifier = myRtPsoProperties->GetShaderIdentifier(shaderName);
  ASSERT(shaderIdentifier);




}
