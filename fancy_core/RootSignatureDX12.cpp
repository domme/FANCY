#include "fancy_core_precompile.h"
#include "RootSignatureDX12.h"

#include "RenderCore_PlatformDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
//---------------------------------------------------------------------------//
  RootSignatureDX12::RootSignatureDX12(const RenderPlatformProperties& someProperties)
  {
    // Keep in sync with resources/Shader/RootSignature.h
    const uint numBindlessTypes = GLOBAL_RESOURCE_NUM;

    const uint numRootParamsNeeded = numBindlessTypes + someProperties.myNumLocalCBuffers + someProperties.myNumLocalBuffers * 2;
    const uint numRangesNeeded = numRootParamsNeeded; // Each param only has one entry and range

    D3D12_ROOT_PARAMETER1* rootParams = static_cast<D3D12_ROOT_PARAMETER1*>(alloca(sizeof(D3D12_ROOT_PARAMETER1) * numRootParamsNeeded));
    D3D12_DESCRIPTOR_RANGE1* ranges = static_cast<D3D12_DESCRIPTOR_RANGE1*>(alloca(sizeof(D3D12_DESCRIPTOR_RANGE1) * numRangesNeeded));
    memset(rootParams, 0, sizeof(D3D12_ROOT_PARAMETER1) * numRootParamsNeeded);
    memset(ranges, 0, sizeof(D3D12_DESCRIPTOR_RANGE1) * numRangesNeeded);

    uint usedRanges = 0;
    uint usedParams = 0;

    const D3D12_DESCRIPTOR_RANGE_FLAGS bindlessRangeFlags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;

    // Global textures
    myRootParamIndex_GlobalResources[GLOBAL_RESOURCE_TEXTURE_2D] = usedParams;
    D3D12_ROOT_PARAMETER1* param = &rootParams[usedParams++];
    D3D12_DESCRIPTOR_RANGE1* range = &ranges[usedRanges++];
    param->DescriptorTable.NumDescriptorRanges = 1;
    param->DescriptorTable.pDescriptorRanges = range;
    range->BaseShaderRegister = 0;
    range->NumDescriptors = someProperties.myNumGlobalTextures;
    range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range->OffsetInDescriptorsFromTableStart = 0;
    range->RegisterSpace = 0;
    range->Flags = bindlessRangeFlags;
    param->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    
    // Global RW Textures
    myRootParamIndex_GlobalResources[GLOBAL_RESOURCE_RW_TEXTURE_2D] = usedParams;
    param = &rootParams[usedParams++];
    range = &ranges[usedRanges++];
    param->DescriptorTable.NumDescriptorRanges = 1;
    param->DescriptorTable.pDescriptorRanges = range;
    range->BaseShaderRegister = 0;
    range->NumDescriptors = someProperties.myNumGlobalTextures;
    range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    range->OffsetInDescriptorsFromTableStart = 0;
    range->RegisterSpace = 1;
    range->Flags = bindlessRangeFlags;
    param->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // Global buffers
    myRootParamIndex_GlobalResources[GLOBAL_RESOURCE_BUFFER] = usedParams;
    param = &rootParams[usedParams++];
    range = &ranges[usedRanges++];
    param->DescriptorTable.NumDescriptorRanges = 1;
    param->DescriptorTable.pDescriptorRanges = range;
    range->BaseShaderRegister = 0;
    range->NumDescriptors = someProperties.myNumGlobalBuffers;
    range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range->OffsetInDescriptorsFromTableStart = 0;
    range->RegisterSpace = 2;
    range->Flags = bindlessRangeFlags;
    param->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // Global rw buffers
    myRootParamIndex_GlobalResources[GLOBAL_RESOURCE_RW_BUFFER] = usedParams;
    param = &rootParams[usedParams++];
    range = &ranges[usedRanges++];
    param->DescriptorTable.NumDescriptorRanges = 1;
    param->DescriptorTable.pDescriptorRanges = range;
    range->BaseShaderRegister = 0;
    range->NumDescriptors = someProperties.myNumGlobalBuffers;
    range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    range->OffsetInDescriptorsFromTableStart = 0;
    range->RegisterSpace = 3;
    range->Flags = bindlessRangeFlags;
    param->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // Global samplers
    myRootParamIndex_GlobalResources[GLOBAL_RESOURCE_SAMPLER] = usedParams;
    param = &rootParams[usedParams++];
    range = &ranges[usedRanges++];
    param->DescriptorTable.NumDescriptorRanges = 1;
    param->DescriptorTable.pDescriptorRanges = range;
    range->BaseShaderRegister = 0;
    range->NumDescriptors = someProperties.myNumGlobalSamplers;
    range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
    range->OffsetInDescriptorsFromTableStart = 0;
    range->RegisterSpace = 4;
    range->Flags = bindlessRangeFlags;
    param->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    myRootParamIndex_LocalBuffers = usedParams;
    myNumLocalBuffers = someProperties.myNumLocalBuffers;

    // Local Buffers
    for (uint i = 0; i < someProperties.myNumLocalBuffers; ++i)
    {
      param = &rootParams[usedParams++];
      param->ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
      param->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
      param->Descriptor.ShaderRegister = i;
      param->Descriptor.RegisterSpace = 5;
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
      param->Descriptor.RegisterSpace = 6;
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
      param->Descriptor.RegisterSpace = 7;
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
