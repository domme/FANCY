#include "fancy_core_precompile.h"
#include "Texture.h"
#include "RenderCore.h"
#include "CommandList.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  Texture::Texture()
    : GpuResource(GpuResourceType::TEXTURE)
    , myIsSwapChainTexture(false)
  {
  }
//---------------------------------------------------------------------------//
  Texture::Texture(GpuResource&& aResource, const TextureProperties& someProperties, bool aIsSwapChainTexture)
    : GpuResource(std::move(aResource))
    , myProperties(someProperties)
    , myIsSwapChainTexture(aIsSwapChainTexture)
  {
  }
//---------------------------------------------------------------------------//
  void Texture::InitTextureData(const TextureSubData* someInitialDatas, uint aNumInitialDatas)
  {
    const uint lastSubresourceIndex = aNumInitialDatas - 1u;
    const SubresourceLocation lastSubresourceLocation = GetSubresourceLocation(lastSubresourceIndex);
    const SubresourceRange subresourceRange(0, lastSubresourceLocation.myMipLevel + 1u,
      0u, lastSubresourceLocation.myArrayIndex + 1u,
      0u, lastSubresourceLocation.myPlaneIndex + 1u);

    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(myProperties.myFormat);
    CommandList* ctx = RenderCore::BeginCommandList(CommandListType::Graphics);
    ctx->UpdateTextureData(this, subresourceRange, someInitialDatas, aNumInitialDatas);
    RenderCore::ExecuteAndFreeCommandList(ctx, SyncMode::BLOCKING);
  }
//---------------------------------------------------------------------------//
  GlobalResourceType TextureView::GetGlobalResourceType(const TextureViewProperties& someViewProps)
  {
    ASSERT(!someViewProps.myIsRenderTarget);

    GlobalResourceType baseType = GLOBAL_RESOURCE_NUM;

    switch (someViewProps.myDimension)
    {
    case GpuResourceDimension::TEXTURE_1D:
      baseType = someViewProps.myIsShaderWritable ? GLOBAL_RESOURCE_RWTEXTURE_1D : GLOBAL_RESOURCE_TEXTURE_1D;
      break;
    case GpuResourceDimension::TEXTURE_2D:
      baseType = someViewProps.myIsShaderWritable ? GLOBAL_RESOURCE_RWTEXTURE_2D : GLOBAL_RESOURCE_TEXTURE_2D;
      break;
    case GpuResourceDimension::TEXTURE_3D:
      baseType = someViewProps.myIsShaderWritable ? GLOBAL_RESOURCE_RWTEXTURE_3D : GLOBAL_RESOURCE_TEXTURE_3D;
      break;
    case GpuResourceDimension::TEXTURE_CUBE:
      ASSERT(!someViewProps.myIsShaderWritable);
      baseType = GLOBAL_RESOURCE_TEXTURE_CUBE;
      break;
    case GpuResourceDimension::NUM: break;
    default: ASSERT(false, "Resource dimension not implemented in the global resource tables");
      return GlobalResourceType::GLOBAL_RESOURCE_NUM;
    }

    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(someViewProps.myFormat);

    return static_cast<GlobalResourceType>(baseType + formatInfo.myIsUintInt);
  }
//---------------------------------------------------------------------------//
}
