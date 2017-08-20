#include "RendererPrerequisites.h"

#include "ComputeContextDX12.h"
#include "CommandListType.h"

#include "GpuProgram.h"
#include "GpuProgramDX12.h"
#include "TextureDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"
#include <malloc.h>

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  std::unordered_map<uint, ID3D12PipelineState*> ComputeContextDX12::ourPSOcache;
//---------------------------------------------------------------------------//
  ComputeContextDX12::ComputeContextDX12()
    : CommandContextDX12(CommandListType::Compute)
  {
    ComputeContextDX12::Reset_Internal();
  }
//---------------------------------------------------------------------------//
  ComputeContextDX12::~ComputeContextDX12()
  {
  }
//---------------------------------------------------------------------------//
  void ComputeContextDX12::Reset()
  {
    Reset_Internal();
  }
//---------------------------------------------------------------------------//
  uint64 ComputeContextDX12::ExecuteAndReset(bool aWaitForCompletion)
  {
    return ExecuteAndReset_Internal(aWaitForCompletion);
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
  void ComputeContextDX12::ClearDepthStencilTarget(Texture* aTexture, float aDepthClear, uint8 aStencilClear, uint32 someClearFlags)
  {
    ClearDepthStencilTarget_Internal(aTexture, aDepthClear, aStencilClear, someClearFlags);
  }
//---------------------------------------------------------------------------//
  void ComputeContextDX12::BindResource(const GpuResource* aResource, ResourceBindingType aBindingType, uint32 aRegisterIndex) const
  {
    ASSERT(myRootSignature != nullptr);

    const GpuResourceDX12* resource = CastGpuResourceDX12(aResource);
    ASSERT(resource != nullptr);

    const uint64 gpuVirtualAddress = resource->GetGpuVirtualAddress();

    switch (aBindingType)
    {
    case ResourceBindingType::SIMPLE: { myCommandList->SetComputeRootShaderResourceView(aRegisterIndex, gpuVirtualAddress); break; }
    case ResourceBindingType::READ_WRITE: { myCommandList->SetComputeRootUnorderedAccessView(aRegisterIndex, gpuVirtualAddress); break; }
    case ResourceBindingType::CONSTANT_BUFFER: { myCommandList->SetComputeRootConstantBufferView(aRegisterIndex, gpuVirtualAddress); break; }
    case ResourceBindingType::RENDER_TARGET:
    case ResourceBindingType::DEPTH_STENCIL_TARGET:
    default: { ASSERT(false); break; }
    }
  }
//---------------------------------------------------------------------------//
  void ComputeContextDX12::BindResourceSet(const GpuResource** someResources, ResourceBindingType* someBindingTypes, uint32 aResourceCount, uint32 aRegisterIndex)
  {
    ASSERT(myRootSignature != nullptr);

    const GpuResourceDX12** resources = static_cast<const GpuResourceDX12**>(alloca(sizeof(const GpuResourceDX12*) * aResourceCount));
    for (uint32 i = 0u; i < aResourceCount; ++i)
    {
      resources[i] = CastGpuResourceDX12(someResources[i]);
      ASSERT(resources[i] != nullptr);
    }

    DescriptorDX12* descriptors = static_cast<DescriptorDX12*>(alloca(sizeof(DescriptorDX12) * aResourceCount));
    for (uint32 i = 0u; i < aResourceCount; ++i)
    {
      switch (someBindingTypes[i])
      {
        case ResourceBindingType::SIMPLE: { const DescriptorDX12* desc = resources[i]->GetSrv(); ASSERT(desc != nullptr); descriptors[i] = *desc; break; }
        case ResourceBindingType::READ_WRITE: { const DescriptorDX12* desc = resources[i]->GetUav(); ASSERT(desc != nullptr); descriptors[i] = *desc; break; }
        case ResourceBindingType::RENDER_TARGET: { const DescriptorDX12* desc = resources[i]->GetRtv(); ASSERT(desc != nullptr); descriptors[i] = *desc; break; }
        case ResourceBindingType::DEPTH_STENCIL_TARGET: { const DescriptorDX12* desc = resources[i]->GetDsv(); ASSERT(desc != nullptr); descriptors[i] = *desc; break; }
        case ResourceBindingType::CONSTANT_BUFFER: { const DescriptorDX12* desc = resources[i]->GetCbv(); ASSERT(desc != nullptr); descriptors[i] = *desc; break; }
        default: { ASSERT(false); break; }
      }
    }

    DescriptorDX12 dynamicRangeStartDescriptor = CopyDescriptorsToDynamicHeapRange(descriptors, aResourceCount);
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
  CommandContextDX12::Reset_Internal();

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
