#include "ShaderResourceInterfaceDX12.h"

#include "MathUtil.h"
#include "RenderCore_PlatformDX12.h"
#include "RenderCore.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  bool ShaderResourceInterfaceDX12::Create(const D3D12_ROOT_SIGNATURE_DESC& anRSDesc, const Microsoft::WRL::ComPtr<ID3D12RootSignature>& aRootSignature /* = nullptr */)
  {
    myInterfaceDesc = CreateDescription(anRSDesc);

    if (aRootSignature != nullptr)
    {
      myRootSignature = aRootSignature;
    }
    else
    {
      Microsoft::WRL::ComPtr<ID3DBlob> signature;
      Microsoft::WRL::ComPtr<ID3DBlob> error;
      Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
      HRESULT success = D3D12SerializeRootSignature(&anRSDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
      ASSERT(success == S_OK, "Failed serializing RootSignature");
      if (success != S_OK)
        return false;

      RenderCore_PlatformDX12* dx12Platform = RenderCore::GetPlatformDX12();
      success = dx12Platform->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
      ASSERT(success == S_OK, "Failed creating RootSignature");

      if (success != S_OK)
        return false;

      myRootSignature = rootSignature;
    }

    return true;
  }
//---------------------------------------------------------------------------//
  ShaderResourceInterfaceDesc ShaderResourceInterfaceDX12::CreateDescription(const D3D12_ROOT_SIGNATURE_DESC& anRSdesc)
  {
    ShaderResourceInterfaceDesc sri;

    for (uint iParam = 0u; iParam < anRSdesc.NumParameters; ++iParam)
    {
      SriElement sriElement;

      const D3D12_ROOT_PARAMETER& param = anRSdesc.pParameters[iParam];
      switch (param.ParameterType)
      {
        case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
          sriElement.myType = SriElementType::DescriptorSet;
          sriElement.myDescriptorSet.myNumElements = param.DescriptorTable.NumDescriptorRanges;
          ASSERT(param.DescriptorTable.NumDescriptorRanges <= kMaxNumDescriptorSetElements);

          for (uint iRange = 0u; iRange < param.DescriptorTable.NumDescriptorRanges; ++iRange)
          {
            const D3D12_DESCRIPTOR_RANGE& range = param.DescriptorTable.pDescriptorRanges[iRange];
            SriDescriptorSetElement& setElement = sriElement.myDescriptorSet.myRangeElements[iRange];
            setElement.myResourceType = locGetResourceType(range.RangeType);
            setElement.myNumElements = range.NumDescriptors;
            setElement.myBindingSlot = range.BaseShaderRegister;
          }
          break;
        case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
          sriElement.myType = SriElementType::Constants;
          sriElement.myConstants.myBindingSlot = param.Constants.ShaderRegister;
          sriElement.myConstants.myNumValues = param.Constants.Num32BitValues;
          break;
        case D3D12_ROOT_PARAMETER_TYPE_CBV:
          sriElement.myType = SriElementType::Descriptor;
          sriElement.myDescriptor.myResourceType = SriResourceType::ConstantBuffer;
          sriElement.myDescriptor.myBindingSlot = param.Descriptor.ShaderRegister;
          break;
        case D3D12_ROOT_PARAMETER_TYPE_SRV:
          sriElement.myType = SriElementType::Descriptor;
          sriElement.myDescriptor.myResourceType = SriResourceType::BufferOrTexture;
          sriElement.myDescriptor.myBindingSlot = param.Descriptor.ShaderRegister;
          break;
        case D3D12_ROOT_PARAMETER_TYPE_UAV:
          sriElement.myType = SriElementType::Descriptor;
          sriElement.myDescriptor.myResourceType = SriResourceType::BufferOrTextureRW;
          sriElement.myDescriptor.myBindingSlot = param.Descriptor.ShaderRegister;
          break;
        default: break;
      }

      sri.myElements.push_back(sriElement);
    }

    sri.myHash = ComputeHash(anRSdesc);
    return sri;
  }
  //---------------------------------------------------------------------------//
  uint ShaderResourceInterfaceDX12::ComputeHash(const D3D12_ROOT_SIGNATURE_DESC& anRSdesc)
  {
    uint hash = 0u;
    MathUtil::hash_combine(hash, anRSdesc.NumParameters);
    for (uint i = 0u; i < anRSdesc.NumParameters; ++i)
      MathUtil::hash_combine(hash, MathUtil::hashFromGeneric(anRSdesc.pParameters[i]));

    MathUtil::hash_combine(hash, anRSdesc.NumStaticSamplers);
    for (uint i = 0u; i < anRSdesc.NumStaticSamplers; ++i)
      MathUtil::hash_combine(hash, MathUtil::hashFromGeneric(anRSdesc.pStaticSamplers[i]));

    MathUtil::hash_combine(hash, anRSdesc.Flags);

    return hash;
  }
//---------------------------------------------------------------------------//
} } }

