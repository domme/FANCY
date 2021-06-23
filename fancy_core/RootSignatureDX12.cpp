#include "fancy_core_precompile.h"
#include "RootSignatureDX12.h"

#include "RenderCore_PlatformDX12.h"
#include "RenderUtils.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
//---------------------------------------------------------------------------//
  RootSignatureDX12::RootSignatureDX12(const RenderPlatformProperties& someProperties)
  {
    // Keep in sync with resources/Shader/RootSignature.h
    const uint numGlobalResourceArrays = GLOBAL_RESOURCE_NUM;

    const uint numRootParamsNeeded = 2 + someProperties.myNumLocalCBuffers + someProperties.myNumLocalBuffers * 2; // buffers and rwbuffers
    const uint numRangesNeeded = numGlobalResourceArrays + numRootParamsNeeded;

    D3D12_ROOT_PARAMETER1* rootParams = static_cast<D3D12_ROOT_PARAMETER1*>(alloca(sizeof(D3D12_ROOT_PARAMETER1) * numRootParamsNeeded));
    D3D12_DESCRIPTOR_RANGE1* ranges = static_cast<D3D12_DESCRIPTOR_RANGE1*>(alloca(sizeof(D3D12_DESCRIPTOR_RANGE1) * numRangesNeeded));
    memset(rootParams, 0, sizeof(D3D12_ROOT_PARAMETER1) * numRootParamsNeeded);
    memset(ranges, 0, sizeof(D3D12_DESCRIPTOR_RANGE1) * numRangesNeeded);

    constexpr D3D12_DESCRIPTOR_RANGE_FLAGS bindlessRangeFlags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;

    uint usedRanges = 0;
    uint usedParams = 0;
    uint offsetInDescriptorHeap = 0;
    

    myRootParamIndex_GlobalResources = usedParams;
    D3D12_ROOT_PARAMETER1* param = &rootParams[usedParams++];
    param->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    param->DescriptorTable.NumDescriptorRanges = GLOBAL_RESOURCE_NUM_NOSAMPLER;
    param->DescriptorTable.pDescriptorRanges = &ranges[usedRanges];

    // SRVs
    uint registerSpace = 0;
    for (uint i = GLOBAL_RESOURCE_SRV_START; i < GLOBAL_RESOURCE_SRV_END; ++i)
    {
      const uint numDescriptors = RenderUtils::GetNumDescriptors(static_cast<GlobalResourceType>(i), someProperties);

      D3D12_DESCRIPTOR_RANGE1* range = &ranges[usedRanges++];
      range->BaseShaderRegister = 0;
      range->NumDescriptors = numDescriptors;
      range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
      range->OffsetInDescriptorsFromTableStart = offsetInDescriptorHeap;
      range->RegisterSpace = registerSpace++;
      range->Flags = bindlessRangeFlags;

      offsetInDescriptorHeap += numDescriptors;
    }

    // UAVs
    registerSpace = 0;
    for (uint i = GLOBAL_RESOURCE_UAV_START; i < GLOBAL_RESOURCE_UAV_END; ++i)
    {
      const uint numDescriptors = RenderUtils::GetNumDescriptors(static_cast<GlobalResourceType>(i), someProperties);

      D3D12_DESCRIPTOR_RANGE1* range = &ranges[usedRanges++];
      range->BaseShaderRegister = 0;
      range->NumDescriptors = numDescriptors;
      range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
      range->OffsetInDescriptorsFromTableStart = offsetInDescriptorHeap;
      range->RegisterSpace = registerSpace++;
      range->Flags = bindlessRangeFlags;

      offsetInDescriptorHeap += numDescriptors;
    }

    // Global samplers
    myRootParamIndex_GlobalSamplers = usedParams;
    param = &rootParams[usedParams++];
    param->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    param->DescriptorTable.NumDescriptorRanges = 1;
    D3D12_DESCRIPTOR_RANGE1* range = &ranges[usedRanges++];
    param->DescriptorTable.pDescriptorRanges = range;
    range->BaseShaderRegister = 0;
    range->NumDescriptors = someProperties.myNumGlobalSamplers;
    range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
    range->OffsetInDescriptorsFromTableStart = 0;
    range->RegisterSpace = 0;
    range->Flags = bindlessRangeFlags;

    myRootParamIndex_LocalBuffers = usedParams;
    myNumLocalBuffers = someProperties.myNumLocalBuffers;

    // Local Buffers
    for (uint i = 0; i < someProperties.myNumLocalBuffers; ++i)
    {
      param = &rootParams[usedParams++];
      param->ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
      param->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
      param->Descriptor.ShaderRegister = i;
      param->Descriptor.RegisterSpace = GLOBAL_RESOURCE_SRV_END;
    }

    myRootParamIndex_LocalRWBuffers = usedParams;
    myNumLocalRWBuffers = someProperties.myNumLocalBuffers;

    // Local RW Buffers
    for (uint i = 0; i < someProperties.myNumLocalBuffers; ++i)
    {
      param = &rootParams[usedParams++];
      param->ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
      param->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
      param->Descriptor.ShaderRegister = i;
      param->Descriptor.RegisterSpace = GLOBAL_RESOURCE_UAV_END;
    }

    myRootParamIndex_LocalCBuffers = usedParams;
    myNumLocalCBuffers = someProperties.myNumLocalCBuffers;

    // Local Cbuffers
    for (uint i = 0; i < someProperties.myNumLocalCBuffers; ++i)
    {
      param = &rootParams[usedParams++];
      param->ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
      param->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
      param->Descriptor.ShaderRegister = i;
      param->Descriptor.RegisterSpace = 0;
    }

    // Guard against accidental override. In this case the "numNeeded" numbers are wrong
    ASSERT(usedParams <= numRootParamsNeeded);
    ASSERT(usedRanges <= numRangesNeeded);

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
    rootSigDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    rootSigDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rootSigDesc.Desc_1_1.NumParameters = usedParams;
    rootSigDesc.Desc_1_1.NumStaticSamplers = 0;
    rootSigDesc.Desc_1_1.pStaticSamplers = nullptr;
    rootSigDesc.Desc_1_1.pParameters = rootParams;

    Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig;
    Microsoft::WRL::ComPtr<ID3DBlob> error;
    HRESULT success = D3D12SerializeVersionedRootSignature(&rootSigDesc, &serializedRootSig, &error);
    if (success != S_OK)
    {
      if (error)
      {
        const char* errorMsg = static_cast<const char*>(error->GetBufferPointer());
        LOG_ERROR("Failed creating root signature: %s", errorMsg);
      }

      ASSERT(false);
    }

    success = RenderCore::GetPlatformDX12()->GetDevice()->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&myRootSignature));
    ASSERT(success == S_OK);
  }
//---------------------------------------------------------------------------//

}

#endif
