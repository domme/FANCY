#include "StdAfx.h"
#include "Fancy.h"
#include "DescriptorHeapPoolDX12.h"
#include "AdapterDX12.h"

#include "TextureSamplerDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  TextureSamplerDX12::TextureSamplerDX12()
  {
  }
//---------------------------------------------------------------------------//
  TextureSamplerDX12::~TextureSamplerDX12()
  {
  }
//---------------------------------------------------------------------------//
  static D3D12_TEXTURE_ADDRESS_MODE locResolveAddressMode(SamplerAddressMode aMode)
  {
    switch(aMode)
    {
      case SamplerAddressMode::WRAP: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
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

    if (aMinMode == SamplerFilterMode::ANISOTROPIC || aMagMode == SamplerFilterMode::ANISOTROPIC)
      return D3D12_FILTER_COMPARISON_ANISOTROPIC;

    switch(aMinMode)
    {
      case SamplerFilterMode::NEAREST: 
        switch (aMagMode)
        {
          case SamplerFilterMode::NEAREST: 
            return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
          case SamplerFilterMode::BILINEAR: 
          case SamplerFilterMode::TRILINEAR:
            return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
        }
        break;
      case SamplerFilterMode::BILINEAR: 
        switch (aMagMode)
        {
          case SamplerFilterMode::NEAREST: 
            return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
          case SamplerFilterMode::BILINEAR:
          case SamplerFilterMode::TRILINEAR: 
            return D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
        }
        break;
      case SamplerFilterMode::TRILINEAR: 
        switch (aMagMode)
        {
          case SamplerFilterMode::NEAREST:
            return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
          case SamplerFilterMode::BILINEAR:
          case SamplerFilterMode::TRILINEAR:
            return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
        }
        break;
    }

    ASSERT(false, "Missing conversion");
    return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
  }
//---------------------------------------------------------------------------//
  void TextureSamplerDX12::Create()
  {
    D3D12_SAMPLER_DESC desc;
    desc.AddressU = locResolveAddressMode(myDescription.addressModeX);
    desc.AddressU = locResolveAddressMode(myDescription.addressModeY);
    desc.AddressW = locResolveAddressMode(myDescription.addressModeZ);
    for (uint32 i = 0u; i < 4u; ++i)
      desc.BorderColor[i] = myDescription.borderColor[i];
    desc.ComparisonFunc = Adapter::toNativeType(myDescription.comparisonFunc);
    desc.Filter = locResolveFilterMode(myDescription.minFiltering, myDescription.magFiltering);
    desc.MaxAnisotropy = myDescription.fMaxAnisotropy;
    desc.MaxLOD = myDescription.fMaxLod;
    desc.MinLOD = myDescription.fMinLod;
    desc.MipLODBias = myDescription.fLodBias;

    RenderCore_PlatformDX12* dx12Platform = RenderCore::GetPlatformDX12();
    myDescriptor = dx12Platform->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

    dx12Platform->GetDevice()->CreateSampler(&desc, myDescriptor.myCpuHandle);
  }
//---------------------------------------------------------------------------//
  bool TextureSamplerDX12::IsCreated()
  {
    return myDescriptor.myCpuHandle.ptr != 0u;
  }
//---------------------------------------------------------------------------//
} } }
