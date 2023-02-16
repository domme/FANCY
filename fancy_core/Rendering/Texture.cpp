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
    const float pixelSizeBytes = formatInfo.mySizeBytes;

    if (pixelSizeBytes > someInitialDatas[0].myPixelSizeBytes)
    {
      // The pixel size is bigger than the size provided by the upload data. 
      // This can happen e.g. if the data is provided as RGB_8 but the rendering-API only supports RGBA_8 formats
      // In this case, we need to manually add some padding per pixel so the upload works

      ASSERT(pixelSizeBytes >= 1.0f); // Code below assumes non-compressed formats

      TextureSubData* newDatas = static_cast<TextureSubData*>(malloc(sizeof(TextureSubData) * aNumInitialDatas));
      for (uint i = 0u; i < aNumInitialDatas; ++i)
      {
        const uint64 width = static_cast<uint64>(someInitialDatas[i].myRowSizeBytes / someInitialDatas[i].myPixelSizeBytes);
        const uint64 height = static_cast<uint64>(someInitialDatas[i].mySliceSizeBytes / someInitialDatas[i].myRowSizeBytes);
        const uint64 depth = static_cast<uint64>(someInitialDatas[i].myTotalSizeBytes / someInitialDatas[i].mySliceSizeBytes);

        const uint64 requiredSizeBytes = static_cast<uint64>(width * height * depth * pixelSizeBytes);
        newDatas[i].myData = (uint8*)malloc(requiredSizeBytes);
        newDatas[i].myTotalSizeBytes = requiredSizeBytes;
        newDatas[i].myPixelSizeBytes = pixelSizeBytes;
        newDatas[i].myRowSizeBytes = static_cast<uint64>(width * pixelSizeBytes);
        newDatas[i].mySliceSizeBytes = static_cast<uint64>(width * height * pixelSizeBytes);

        const float oldPixelSizeBytes = someInitialDatas[i].myPixelSizeBytes;
        const uint64 numPixels = width * height * depth;
        for (uint64 iPixel = 0u; iPixel < numPixels; ++iPixel)
        {
          // Copy the smaller pixel to the new location
          uint8* destPixelDataPtr = newDatas[i].myData + iPixel * static_cast<int>(pixelSizeBytes);
          uint8* srcPixelDataPtr = someInitialDatas[i].myData + iPixel * static_cast<int>(oldPixelSizeBytes);
          memcpy(destPixelDataPtr, srcPixelDataPtr, oldPixelSizeBytes);

          // Add the padding value
          const uint kPaddingValue = ~0u;
          memset(destPixelDataPtr + static_cast<int>(oldPixelSizeBytes), kPaddingValue, static_cast<int>(pixelSizeBytes) - static_cast<int>(oldPixelSizeBytes));
        }
      }

      CommandList* ctx = RenderCore::BeginCommandList(CommandListType::Graphics);
      ctx->UpdateTextureData(this, subresourceRange, newDatas, aNumInitialDatas);
      RenderCore::ExecuteAndFreeCommandList(ctx, SyncMode::BLOCKING);

      for (uint i = 0u; i < aNumInitialDatas; ++i)
        free(newDatas[i].myData);

      free(newDatas);
    }
    else
    {
      CommandList* ctx = RenderCore::BeginCommandList(CommandListType::Graphics);
      ctx->UpdateTextureData(this, subresourceRange, someInitialDatas, aNumInitialDatas);
      RenderCore::ExecuteAndFreeCommandList(ctx, SyncMode::BLOCKING);
    }
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
