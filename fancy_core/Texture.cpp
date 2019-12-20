#include "fancy_core_precompile.h"
#include "Texture.h"
#include "RenderCore.h"
#include "CommandList.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  Texture::Texture()
    : GpuResource(GpuResourceCategory::TEXTURE)
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
    const uint pixelSizeBytes = formatInfo.mySizeBytes;

    if (pixelSizeBytes > someInitialDatas[0].myPixelSizeBytes)
    {
      // The pixel size is bigger than the size provided by the upload data. 
      // This can happen e.g. if the data is provided as RGB_8 but the rendering-API only supports RGBA_8 formats
      // In this case, we need to manually add some padding per pixel so the upload works

      TextureSubData* newDatas = static_cast<TextureSubData*>(malloc(sizeof(TextureSubData) * aNumInitialDatas));
      for (uint i = 0u; i < aNumInitialDatas; ++i)
      {
        const uint64 width = someInitialDatas[i].myRowSizeBytes / someInitialDatas[i].myPixelSizeBytes;
        const uint64 height = someInitialDatas[i].mySliceSizeBytes / someInitialDatas[i].myRowSizeBytes;
        const uint64 depth = someInitialDatas[i].myTotalSizeBytes / someInitialDatas[i].mySliceSizeBytes;

        const uint64 requiredSizeBytes = width * height * depth * pixelSizeBytes;
        newDatas[i].myData = (uint8*)malloc(requiredSizeBytes);
        newDatas[i].myTotalSizeBytes = requiredSizeBytes;
        newDatas[i].myPixelSizeBytes = pixelSizeBytes;
        newDatas[i].myRowSizeBytes = width * pixelSizeBytes;
        newDatas[i].mySliceSizeBytes = width * height * pixelSizeBytes;

        const uint64 oldPixelSizeBytes = someInitialDatas[i].myPixelSizeBytes;
        const uint64 numPixels = width * height * depth;
        for (uint64 iPixel = 0u; iPixel < numPixels; ++iPixel)
        {
          // Copy the smaller pixel to the new location
          uint8* destPixelDataPtr = newDatas[i].myData + iPixel * pixelSizeBytes;
          uint8* srcPixelDataPtr = someInitialDatas[i].myData + iPixel * oldPixelSizeBytes;
          memcpy(destPixelDataPtr, srcPixelDataPtr, oldPixelSizeBytes);

          // Add the padding value
          const uint kPaddingValue = ~0u;
          memset(destPixelDataPtr + oldPixelSizeBytes, kPaddingValue, pixelSizeBytes - oldPixelSizeBytes);
        }
      }

      CommandList* ctx = RenderCore::BeginCommandList(CommandListType::Graphics);
      ctx->UpdateTextureData(this, subresourceRange, newDatas, aNumInitialDatas);
      ctx->ResourceBarrier(this, GpuResourceState::WRITE_COPY_DEST, myStateTracking.myDefaultState);
      RenderCore::ExecuteAndFreeCommandList(ctx, SyncMode::BLOCKING);

      for (uint i = 0u; i < aNumInitialDatas; ++i)
        free(newDatas[i].myData);

      free(newDatas);
    }
    else
    {
      CommandList* ctx = RenderCore::BeginCommandList(CommandListType::Graphics);
      ctx->UpdateTextureData(this, subresourceRange, someInitialDatas, aNumInitialDatas);
      ctx->ResourceBarrier(this, GpuResourceState::WRITE_COPY_DEST, myStateTracking.myDefaultState);
      RenderCore::ExecuteAndFreeCommandList(ctx, SyncMode::BLOCKING);
    }
  }
//---------------------------------------------------------------------------//
}
