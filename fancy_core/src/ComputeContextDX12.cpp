#include "RendererPrerequisites.h"

#include "ComputeContextDX12.h"
#include "CommandListType.h"

#include "GpuBuffer.h"
#include "GpuBufferDX12.h"

#include "GpuProgram.h"
#include "GpuProgramDX12.h"
#include "TextureDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  std::unordered_map<uint, ID3D12PipelineState*> ComputeContextDX12::ourPSOcache;
//---------------------------------------------------------------------------//
  ComputeContextDX12::ComputeContextDX12()
    : CommandContextBaseDX12(CommandListType::Compute)
  {
    ComputeContextDX12::Reset_Internal();
  }
//---------------------------------------------------------------------------//
  ComputeContextDX12::~ComputeContextDX12()
  {
  }
//---------------------------------------------------------------------------//
  D3D12_COMPUTE_PIPELINE_STATE_DESC ComputeContextDX12::GetNativePSOdesc(const ComputePipelineState& aState)
  {
    D3D12_COMPUTE_PIPELINE_STATE_DESC desc;
    memset(&desc, 0u, sizeof(desc));

    if (aState.myGpuProgram != nullptr)
    {
      const GpuProgramDX12* gpuProgramDx12 = 
        static_cast<const GpuProgramDX12*>(aState.myGpuProgram);

      desc.pRootSignature = gpuProgramDx12->GetRootSignature();
      desc.CS = gpuProgramDx12->getNativeByteCode();
    }

    desc.NodeMask = 0u;

    return desc;
  }
//---------------------------------------------------------------------------//
  void ComputeContextDX12::ClearRenderTarget(Texture* aTexture, const float* aColor)
  {
    ClearRenderTarget_Internal(aTexture, aColor);
  }
//---------------------------------------------------------------------------//
  void ComputeContextDX12::ClearDepthStencilTarget(Texture* aTexture, float aDepthClear, uint8 aStencilClear, uint32 someClearFlags) const
  {
    ClearDepthStencilTarget_Internal(aTexture, aDepthClear, aStencilClear, someClearFlags);
  }
//---------------------------------------------------------------------------//
void ComputeContextDX12::SetReadTexture(const Texture* aTexture, uint32 aRegisterIndex) const
{
  ASSERT(myRootSignature != nullptr);
  myCommandList->SetComputeRootShaderResourceView(aRegisterIndex,
    static_cast<const TextureDX12*>(aTexture)->GetGpuVirtualAddress());
}
//---------------------------------------------------------------------------//
void ComputeContextDX12::SetWriteTexture(const Texture* aTexture, uint32 aRegisterIndex) const
{
  ASSERT(myRootSignature != nullptr);
  myCommandList->SetComputeRootUnorderedAccessView(aRegisterIndex, 
    static_cast<const TextureDX12*>(aTexture)->GetGpuVirtualAddress());
}
//---------------------------------------------------------------------------//
void ComputeContextDX12::SetReadBuffer(const GpuBuffer* aBuffer, uint32 aRegisterIndex) const
{
  ASSERT(myRootSignature != nullptr);
  myCommandList->SetComputeRootShaderResourceView(aRegisterIndex, 
    static_cast<const GpuBufferDX12*>(aBuffer)->GetGpuVirtualAddress());
}
//---------------------------------------------------------------------------//
void ComputeContextDX12::SetConstantBuffer(const GpuBuffer* aConstantBuffer, uint32 aRegisterIndex) const
{
  ASSERT(myRootSignature != nullptr);
  myCommandList->SetComputeRootConstantBufferView(aRegisterIndex, 
    static_cast<const GpuBufferDX12*>(aConstantBuffer)->GetGpuVirtualAddress());
}
//---------------------------------------------------------------------------//
  void ComputeContextDX12::SetMultipleResources(const Descriptor* someResources, uint32 aResourceCount, uint32 aRegisterIndex)
  {
    ASSERT(myRootSignature != nullptr);

    DescriptorDX12 dynamicRangeStartDescriptor =
      CopyDescriptorsToDynamicHeapRange(someResources, aResourceCount);

    myCommandList->SetComputeRootDescriptorTable(aRegisterIndex, dynamicRangeStartDescriptor.myGpuHandle);
  }
//---------------------------------------------------------------------------//
  void ComputeContextDX12::SetComputeProgram(const GpuProgram* aProgram)
  {
    ASSERT(aProgram->getShaderStage() == ShaderStage::COMPUTE);

    if (myComputePipelineState.myGpuProgram != aProgram)
    {
      myComputePipelineState.myGpuProgram = aProgram;
      myComputePipelineState.myIsDirty = true;
    }

    const GpuProgramDX12* programDx12 = static_cast<const GpuProgramDX12*>(aProgram);

    if (myRootSignature != programDx12->GetRootSignature())
    {
      myRootSignature = programDx12->GetRootSignature();
      myCommandList->SetComputeRootSignature(myRootSignature);
    }
  }
//---------------------------------------------------------------------------//
  void ComputeContextDX12::Dispatch(size_t aThreadGroupCountX, size_t aThreadGroupCountY, size_t aThreadGroupCountZ)
  {
    ApplyPipelineState();

    KickoffResourceBarriers();
    myCommandList->Dispatch(aThreadGroupCountX, aThreadGroupCountY, aThreadGroupCountZ);
  }
//---------------------------------------------------------------------------//
void ComputeContextDX12::Reset_Internal()
{
  CommandContextBaseDX12::Reset_Internal();

  myComputePipelineState = ComputePipelineState();
}
//---------------------------------------------------------------------------//
void ComputeContextDX12::ApplyPipelineState()
{
  if (!myComputePipelineState.myIsDirty)
    return;

  myComputePipelineState.myIsDirty = false;

  uint requestedHash = myComputePipelineState.GetHash();

  ID3D12PipelineState* pso = nullptr;

  auto cachedPSOIter = ourPSOcache.find(requestedHash);
  if (cachedPSOIter != ourPSOcache.end())
  {
    pso = cachedPSOIter->second;
  }
  else
  {
    const D3D12_COMPUTE_PIPELINE_STATE_DESC& psoDesc = GetNativePSOdesc(myComputePipelineState);
    HRESULT result = RenderCore::GetPlatformDX12()->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pso));
    ASSERT(result == S_OK, "Error creating compute PSO");

    ourPSOcache[requestedHash] = pso;
  }
  myCommandList->SetPipelineState(pso);
}
//---------------------------------------------------------------------------//
} } }
