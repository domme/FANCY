#include "RendererPrerequisites.h"

#include "ComputeContextDX12.h"
#include "CommandListType.h"
#include "GpuProgram.h"
#include "ShaderResourceInterface.h"
#include "Renderer.h"
#include "GpuBuffer.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  ComputePipelineState::ComputePipelineState()
    : myIsDirty(true)
    , myGpuProgram(nullptr)
  {
  }
//---------------------------------------------------------------------------//
  uint ComputePipelineState::GetHash()
  {
    uint hash = 0u;
    MathUtil::hash_combine(hash, reinterpret_cast<uint64>(myGpuProgram));

    if (myGpuProgram != nullptr)
      MathUtil::hash_combine(hash, reinterpret_cast<uint64>(myGpuProgram->getNativeData().Get()));

    return hash;
  }
//---------------------------------------------------------------------------//
  D3D12_COMPUTE_PIPELINE_STATE_DESC ComputePipelineState::GetNativePSOdesc()
  {
    D3D12_COMPUTE_PIPELINE_STATE_DESC desc;
    memset(&desc, 0u, sizeof(desc));

    if (myGpuProgram != nullptr)
    {
      desc.pRootSignature = myGpuProgram->GetRootSignature();
      desc.CS = myGpuProgram->getNativeByteCode();
    }

    desc.NodeMask = 0u;

    return desc;
  }
//---------------------------------------------------------------------------//
  std::unordered_map<uint, ID3D12PipelineState*> ComputeContextDX12::ourPSOcache;
//---------------------------------------------------------------------------//
ComputeContextDX12::ComputeContextDX12()
  : CommandContext(CommandListType::Compute)
{
  ComputeContextDX12::ResetInternal();
}
//---------------------------------------------------------------------------//
ComputeContextDX12::~ComputeContextDX12()
{
}
//---------------------------------------------------------------------------//
void ComputeContextDX12::SetReadTexture(const Texture* aTexture, uint32 aRegisterIndex) const
{
  ASSERT(myRootSignature != nullptr);
  myCommandList->SetComputeRootShaderResourceView(aRegisterIndex, aTexture->GetGpuVirtualAddress());
}
//---------------------------------------------------------------------------//
void ComputeContextDX12::SetWriteTexture(const Texture* aTexture, uint32 aRegisterIndex) const
{
  ASSERT(myRootSignature != nullptr);
  myCommandList->SetComputeRootUnorderedAccessView(aRegisterIndex, aTexture->GetGpuVirtualAddress());
}
//---------------------------------------------------------------------------//
void ComputeContextDX12::SetReadBuffer(const GpuBuffer* aBuffer, uint32 aRegisterIndex) const
{
  ASSERT(myRootSignature != nullptr);
  myCommandList->SetComputeRootShaderResourceView(aRegisterIndex, aBuffer->GetGpuVirtualAddress());
}
//---------------------------------------------------------------------------//
void ComputeContextDX12::SetConstantBuffer(const GpuBuffer* aConstantBuffer, uint32 aRegisterIndex) const
{
  ASSERT(myRootSignature != nullptr);
  myCommandList->SetComputeRootConstantBufferView(aRegisterIndex, aConstantBuffer->GetGpuVirtualAddress());
}
//---------------------------------------------------------------------------//
void ComputeContextDX12::SetMultipleResources(const Descriptor* someResources, uint32 aResourceCount, uint32 aRegisterIndex)
{
  ASSERT(myRootSignature != nullptr);
  ASSERT(aResourceCount > 0u);

  D3D12_DESCRIPTOR_HEAP_TYPE heapType = someResources[0].myHeapType;
  DescriptorHeapDX12* dynamicHeap = myDynamicShaderVisibleHeaps[heapType];

  if (dynamicHeap == nullptr)
  {
    dynamicHeap = RenderCore::AllocateDynamicDescriptorHeap(aResourceCount, heapType);
    SetDescriptorHeap(heapType, dynamicHeap);
  }

  uint32 startOffset = dynamicHeap->GetNumAllocatedDescriptors();
  for (uint32 i = 0u; i < aResourceCount; ++i)
  {
    DescriptorDX12 destDescriptor = dynamicHeap->AllocateDescriptor();

    RenderCore::GetDevice()->CopyDescriptorsSimple(1, destDescriptor.myCpuHandle,
      someResources[i].myCpuHandle, heapType);
  }

  myCommandList->SetComputeRootDescriptorTable(aRegisterIndex, dynamicHeap->GetDescriptor(startOffset).myGpuHandle);
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

  if (myRootSignature != aProgram->GetRootSignature())
  {
    myRootSignature = aProgram->GetRootSignature();
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
void ComputeContextDX12::ResetInternal()
{
  CommandContext::ResetInternal();
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
    const D3D12_COMPUTE_PIPELINE_STATE_DESC& psoDesc = myComputePipelineState.GetNativePSOdesc();
    HRESULT result = RenderCore::GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pso));
    ASSERT(result == S_OK, "Error creating compute PSO");

    ourPSOcache[requestedHash] = pso;
  }
  myCommandList->SetPipelineState(pso);
}
//---------------------------------------------------------------------------//
} } }
