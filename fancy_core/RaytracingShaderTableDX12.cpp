#include "fancy_core_precompile.h"
#include "RaytracingShaderTableDX12.h"

#include "GpuBuffer.h"
#include "RaytracingPipelineStateDX12.h"
#include "RenderCore_PlatformDX12.h"

using namespace Fancy;

RaytracingShaderTableDX12::RaytracingShaderTableDX12(const RaytracingShaderTableProperties& someProps, const SharedPtr<RaytracingPipelineState>& anRtPso)
  : RaytracingShaderTable(someProps, anRtPso)
{
  auto device = RenderCore::GetPlatformDX12()->GetDevice();
  const RaytracingPipelineStateDX12* rtPsoDx12 = (RaytracingPipelineStateDX12*)anRtPso.get();
  ASSERT_HRESULT(rtPsoDx12->myStateObject.As(&myRtPsoProperties));
}

void RaytracingShaderTableDX12::GetShaderRecordDataInternal(uint /*aShaderIndexInRtPso*/, const RaytracingPipelineStateProperties::ShaderEntry& aShaderEntry, eastl::fixed_vector<uint8, 64>& someDataOut)
{
  void* shaderIdentifier = myRtPsoProperties->GetShaderIdentifier(aShaderEntry.myUniqueMainFunctionName.c_str());
  ASSERT(shaderIdentifier);

  someDataOut.resize(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
  memcpy(someDataOut.data(), shaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
}

void RaytracingShaderTableDX12::GetShaderRecordDataInternal(uint aShaderIndexInRtPso, const RaytracingPipelineStateProperties::HitGroup& aShaderEntry, eastl::fixed_vector<uint8, 64>& someDataOut)
{
  void* shaderIdentifier = myRtPsoProperties->GetShaderIdentifier(aShaderEntry.myName.c_str());
  ASSERT(shaderIdentifier);

  someDataOut.resize(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
  memcpy(someDataOut.data(), shaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
}
