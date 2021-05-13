#include "fancy_core_precompile.h"
#include "RootSignatureCacheDX12.h"

#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
  RootSignatureCacheDX12::RootSignatureCacheDX12(const RenderPlatformProperties& someProperties)
  {
    CreateBindlessRootSig(someProperties);
  }

  void RootSignatureCacheDX12::CreateBindlessRootSig(const RenderPlatformProperties& someProperties)
  {
    const uint numRootParamsNeeded =
      5 +  // Bindless
      3 +  // Local no cbuffer
      someProperties.myNumLocalCBuffers;

    const uint numRangesNeeded =
      5 +
      3 +
      someProperties.myNumLocalCBuffers;

    const uint numDescriptorTablesNeeded =
      5 +
      3;

    D3D12_ROOT_PARAMETER1* rootParams = static_cast<D3D12_ROOT_PARAMETER1*>(alloca(sizeof(D3D12_ROOT_PARAMETER1) * numRootParamsNeeded));
    D3D12_DESCRIPTOR_RANGE1* ranges = static_cast<D3D12_DESCRIPTOR_RANGE1*>(alloca(sizeof(D3D12_DESCRIPTOR_RANGE1) * numRangesNeeded));

    uint usedRanges = 0;
    uint usedParams = 0;

    // Bindless textures
    D3D12_ROOT_PARAMETER1* param = &rootParams[usedParams++];
    D3D12_DESCRIPTOR_RANGE1* range = &ranges[usedRanges++];
    param->DescriptorTable.NumDescriptorRanges = 1;
    param->DescriptorTable.pDescriptorRanges = range;
    range->BaseShaderRegister = 0;
    range->NumDescriptors = someProperties.myNumBindlessTexturesRWTextures;
    range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range->OffsetInDescriptorsFromTableStart = 0;
    range->RegisterSpace = 0;
    param->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // Bindless RW Textures
    param = &rootParams[usedParams++];
    range = &ranges[usedRanges++];
    param->DescriptorTable.NumDescriptorRanges = 1;
    param->DescriptorTable.pDescriptorRanges = range;
    range->BaseShaderRegister = 0;
    range->NumDescriptors = someProperties.myNumBindlessTexturesRWTextures;
    range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    range->OffsetInDescriptorsFromTableStart = 0;
    range->RegisterSpace = 1;
    param->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // Bindless buffers
    param = &rootParams[usedParams++];
    range = &ranges[usedRanges++];
    param->DescriptorTable.NumDescriptorRanges = 1;
    param->DescriptorTable.pDescriptorRanges = range;
    range->BaseShaderRegister = 0;
    range->NumDescriptors = someProperties.myNumBindlessBuffersRWBuffers;
    range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range->OffsetInDescriptorsFromTableStart = 0;
    range->RegisterSpace = 2;
    param->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // Bindless rw buffers
    param = &rootParams[usedParams++];
    range = &ranges[usedRanges++];
    param->DescriptorTable.NumDescriptorRanges = 1;
    param->DescriptorTable.pDescriptorRanges = range;
    range->BaseShaderRegister = 0;
    range->NumDescriptors = someProperties.myNumBindlessBuffersRWBuffers;
    range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    range->OffsetInDescriptorsFromTableStart = 0;
    range->RegisterSpace = 3;
    param->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // Bindless samplers
    param = &rootParams[usedParams++];
    range = &ranges[usedRanges++];
    param->DescriptorTable.NumDescriptorRanges = 1;
    param->DescriptorTable.pDescriptorRanges = range;
    range->BaseShaderRegister = 0;
    range->NumDescriptors = someProperties.myNumBindlessSamplers;
    range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
    range->OffsetInDescriptorsFromTableStart = 0;
    range->RegisterSpace = 4;
    param->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // Local SRVs
    param = &rootParams[usedParams++];
    range = &ranges[usedRanges++];
    param->DescriptorTable.NumDescriptorRanges = 1;
    param->DescriptorTable.pDescriptorRanges = range;
    range->BaseShaderRegister = 0;
    range->NumDescriptors = someProperties.myNumLocalSRVsUAVs;
    range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range->OffsetInDescriptorsFromTableStart = 0;
    range->RegisterSpace = 5;
    param->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // Local UAVs
    param = &rootParams[usedParams++];
    range = &ranges[usedRanges++];
    param->DescriptorTable.NumDescriptorRanges = 1;
    param->DescriptorTable.pDescriptorRanges = range;
    range->BaseShaderRegister = 0;
    range->NumDescriptors = someProperties.myNumLocalSRVsUAVs;
    range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    range->OffsetInDescriptorsFromTableStart = 0;
    range->RegisterSpace = 6;
    param->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // Local Samplers
    param = &rootParams[usedParams++];
    range = &ranges[usedRanges++];
    param->DescriptorTable.NumDescriptorRanges = 1;
    param->DescriptorTable.pDescriptorRanges = range;
    range->BaseShaderRegister = 0;
    range->NumDescriptors = someProperties.myNumLocalSamplers;
    range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
    range->OffsetInDescriptorsFromTableStart = 0;
    range->RegisterSpace = 7;
    param->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // Local Cbuffers
    for (uint i = 0; i < someProperties.myNumLocalCBuffers; ++i)
    {
      param = &rootParams[usedParams++];
      param->ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
      param->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
      param->Descriptor.ShaderRegister = i;
      param->Descriptor.RegisterSpace = 8;
    }

    ASSERT(usedParams <= numRootParamsNeeded);
    ASSERT(usedRanges <= numRangesNeeded);

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
    rootSigDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    rootSigDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rootSigDesc.Desc_1_1.NumParameters = usedParams;
    rootSigDesc.Desc_1_1.NumStaticSamplers = 0;
    rootSigDesc.Desc_1_1.pStaticSamplers = nullptr;
    rootSigDesc.Desc_1_1.pParameters = rootParams;

    Microsoft::WRL::ComPtr<ID3DBlob> signature;
    Microsoft::WRL::ComPtr<ID3DBlob> error;
    HRESULT success = D3D12SerializeVersionedRootSignature(&rootSigDesc, &signature, &error);
    ASSERT(success == S_OK);

    // Create the serialized root signature
    ID3D12Device8* d3dDevice = RenderCore::GetPlatformDX12()->GetDevice();
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    success = d3dDevice->CreateRootSignature(0, signature.Get(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
    ASSERT(success == S_OK);

    myBindlessDefaultRootSignature.myLayout.reset(new RootSignatureLayoutDX12(rootSigDesc.Desc_1_1));
    myBindlessDefaultRootSignature.myRootSignature = rootSignature;

    myCache[myBindlessDefaultRootSignature.myLayout->GetHash()] = myBindlessDefaultRootSignature;
  }

  bool RootSignatureCacheDX12::ReplaceWithCached(SharedPtr<RootSignatureLayoutDX12>& aLayout, Microsoft::WRL::ComPtr<ID3D12RootSignature>& aRootSignature)
  {
    const uint64 hash = aLayout->GetHash();
    auto it = myCache.find(hash);

    if (it != myCache.end())
    {
      aLayout = it->second.myLayout;
      aRootSignature = it->second.myRootSignature;
      return true;
    }

    myCache[hash] = { aRootSignature, aLayout };
    return false;
  }
}

#endif