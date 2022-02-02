#include "fancy_core_precompile.h"
#include "TextureSamplerDX12.h"

#include "Rendering/RenderCore.h"
#include "RenderCore_PlatformDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy 
{
  namespace Priv_TextureSamplerDX12
  {
//---------------------------------------------------------------------------//
    static D3D12_TEXTURE_ADDRESS_MODE locResolveAddressMode(SamplerAddressMode aMode)
    {
      switch (aMode)
      {
      case SamplerAddressMode::CLAMP_EDGE: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
      case SamplerAddressMode::MIRROR_CLAMP_EDGE: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
      case SamplerAddressMode::CLAMP_BORDER: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
      case SamplerAddressMode::REPEAT: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
      case SamplerAddressMode::MIRROR_REPEAT: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
      default: ASSERT(false); return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
      }
    }
//---------------------------------------------------------------------------//
    static D3D12_FILTER locResolveFilterMode(SamplerFilterMode aMinMode, SamplerFilterMode aMagMode)
    {
      // Rules for conversion: Mipmap-filtering is controlled only by the MinMode - the MagMode doesn't influence it
      // If any filter mode is anisotropic, all modes are anisotropic

      // TODO: ADD support for comparison and MIN/MAX filters

      if (aMinMode == SamplerFilterMode::ANISOTROPIC || aMagMode == SamplerFilterMode::ANISOTROPIC)
        return D3D12_FILTER_COMPARISON_ANISOTROPIC;

      switch (aMinMode)
      {
      case SamplerFilterMode::NEAREST:
        switch (aMagMode)
        {
        case SamplerFilterMode::NEAREST:
          return D3D12_FILTER_MIN_MAG_MIP_POINT;
        case SamplerFilterMode::BILINEAR:
        case SamplerFilterMode::TRILINEAR:
          return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
        }
        break;
      case SamplerFilterMode::BILINEAR:
        switch (aMagMode)
        {
        case SamplerFilterMode::NEAREST:
          return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        case SamplerFilterMode::BILINEAR:
        case SamplerFilterMode::TRILINEAR:
          return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        }
        break;
      case SamplerFilterMode::TRILINEAR:
        switch (aMagMode)
        {
        case SamplerFilterMode::NEAREST:
          return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        case SamplerFilterMode::BILINEAR:
        case SamplerFilterMode::TRILINEAR:
          return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        }
        break;
      }

      ASSERT(false, "Missing conversion");
      return D3D12_FILTER_MIN_MAG_MIP_POINT;
    }
//---------------------------------------------------------------------------//
    glm::vec4 locResolveBorderColor(SamplerBorderColor aColor)
    {
      switch(aColor)
      {
        case SamplerBorderColor::FLOAT_TRANSPARENT_BLACK: return glm::vec4(0.0f);
        case SamplerBorderColor::INT_TRANSPARENT_BLACK: return glm::vec4(0.0f);
        case SamplerBorderColor::FLOAT_OPAQUE_BLACK: return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        case SamplerBorderColor::INT_OPAQUE_BLACK: return glm::vec4(0.0f, 0.0f, 0.0f, (float)INT_MAX);
        case SamplerBorderColor::FLOAT_OPAQUE_WHITE: return glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        case SamplerBorderColor::INT_OPAQUE_WHITE: return glm::vec4((float)INT_MAX);
        default: ASSERT(false); return glm::vec4(0.0f);
      }
    }
 //---------------------------------------------------------------------------//
  }
//---------------------------------------------------------------------------//
  TextureSamplerDX12::TextureSamplerDX12(const TextureSamplerProperties& someProperties)
    : TextureSampler(someProperties)
  {
    D3D12_SAMPLER_DESC desc;
    desc.AddressU = Priv_TextureSamplerDX12::locResolveAddressMode(myProperties.myAddressModeX);
    desc.AddressV = Priv_TextureSamplerDX12::locResolveAddressMode(myProperties.myAddressModeY);
    desc.AddressW = Priv_TextureSamplerDX12::locResolveAddressMode(myProperties.myAddressModeZ);
    const glm::vec4 borderColor = Priv_TextureSamplerDX12::locResolveBorderColor(myProperties.myBorderColor);
    for (uint i = 0u; i < 4u; ++i)
      desc.BorderColor[i] = borderColor[i];
    desc.ComparisonFunc = RenderCore_PlatformDX12::ResolveCompFunc(myProperties.myComparisonFunc);
    desc.Filter = Priv_TextureSamplerDX12::locResolveFilterMode(myProperties.myMinFiltering, myProperties.myMagFiltering);
    desc.MaxAnisotropy = glm::min(RenderCore::GetPlatformCaps().myMaxTextureAnisotropy, myProperties.myMaxAnisotropy);
    desc.MaxLOD = myProperties.myMaxLod;
    desc.MinLOD = myProperties.myMinLod;
    desc.MipLODBias = myProperties.myLodBias;

    RenderCore_PlatformDX12* dx12Platform = RenderCore::GetPlatformDX12();

    myDescriptor = dx12Platform->AllocateShaderVisibleDescriptorForGlobalResource(GLOBAL_RESOURCE_SAMPLER);
    myGlobalDescriptorIndex = myDescriptor.myGlobalResourceIndex;

    dx12Platform->GetDevice()->CreateSampler(&desc, myDescriptor.myCpuHandle);
  }
//---------------------------------------------------------------------------//
  TextureSamplerDX12::~TextureSamplerDX12()
  {
    RenderCore_PlatformDX12* dx12Platform = RenderCore::GetPlatformDX12();
    dx12Platform->FreeDescriptor(myDescriptor);
  }
//---------------------------------------------------------------------------//
}

#endif