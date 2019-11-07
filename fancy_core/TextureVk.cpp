#include "fancy_core_precompile.h"
#include "TextureVk.h"
#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"
#include "GpuResourceDataVk.h"
#include "GpuResourceViewDataVk.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  TextureVk::TextureVk(GpuResource&& aResource, const TextureProperties& someProperties)
    : Texture(std::move(aResource), someProperties)
  {
  }
//---------------------------------------------------------------------------//
  TextureVk::~TextureVk()
  {
    Destroy();
  }
//---------------------------------------------------------------------------//
  bool TextureVk::IsValid() const
  {
    const GpuResourceDataVk* const nativeData = GetData();
    return nativeData != nullptr && nativeData->myImage != nullptr;
  }
//---------------------------------------------------------------------------//
  void TextureVk::SetName(const char* aName)
  {
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

    GpuResourceDataVk* const nativeData = GetData();

    VkDebugUtilsObjectNameInfoEXT nameInfo = {};
    nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    nameInfo.objectHandle = (uint64)nativeData->myImage;
    nameInfo.objectType = VK_OBJECT_TYPE_IMAGE;
    nameInfo.pObjectName = aName;
    ASSERT_VK_RESULT(platformVk->VkSetDebugUtilsObjectNameEXT(platformVk->myDevice, &nameInfo));
  }
//---------------------------------------------------------------------------//
  void TextureVk::Create(const TextureProperties& someProperties, const char* aName, const TextureSubData* someInitialDatas, uint aNumInitialDatas)
  {
    VK_MISSING_IMPLEMENTATION();
  }
//---------------------------------------------------------------------------//
  void TextureVk::GetSubresourceLayout(const TextureSubLocation& aStartSubLocation, uint aNumSubDatas, DynamicArray<TextureSubLayout>& someLayoutsOut, DynamicArray<uint64>& someOffsetsOut, uint64& aTotalSizeOut) const
  {
    VK_MISSING_IMPLEMENTATION();
  }
//---------------------------------------------------------------------------//
  GpuResourceDataVk* TextureVk::GetData() const
  {
    return myNativeData.IsEmpty() ? nullptr : myNativeData.To<GpuResourceDataVk*>();
  }
//---------------------------------------------------------------------------//
  void TextureVk::Destroy()
  {
    if (IsValid())
    {
      const GpuResourceDataVk* const dataVk = GetData();
      RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
      vkDestroyImage(platformVk->myDevice, dataVk->myImage, nullptr);

      delete dataVk;
    }

    myNativeData.Clear();
    myStateTracking = GpuResourceStateTracking();
    myProperties = TextureProperties();
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  namespace Priv_TextureViewVk
  {
    VkImageViewType locGetImageViewType(GpuResourceDimension aDimension)
    {
      switch (aDimension)
      {
        case GpuResourceDimension::TEXTURE_1D: return VK_IMAGE_VIEW_TYPE_1D;
        case GpuResourceDimension::TEXTURE_2D: return VK_IMAGE_VIEW_TYPE_2D;
        case GpuResourceDimension::TEXTURE_3D: return VK_IMAGE_VIEW_TYPE_3D;
        case GpuResourceDimension::TEXTURE_CUBE: return VK_IMAGE_VIEW_TYPE_CUBE;
        case GpuResourceDimension::TEXTURE_1D_ARRAY: return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
        case GpuResourceDimension::TEXTURE_2D_ARRAY: return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        case GpuResourceDimension::TEXTURE_CUBE_ARRAY: return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        default: 
          ASSERT(false, "Unrecognized or unsupported GpuResourceDimension");
          return VK_IMAGE_VIEW_TYPE_2D;
      }
    }
//---------------------------------------------------------------------------//
    VkImageView locCreateImageView(Texture* aTexture, const TextureViewProperties& someProperties)
    {
      const TextureVk* textureVk = static_cast<const TextureVk*>(aTexture);
      const GpuResourceDataVk* const dataVk = textureVk->GetData();

      const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(someProperties.myFormat);
      const TextureSubRange& subresourceRange = someProperties.mySubresourceRange;

      VkImageViewCreateInfo info;
      info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      info.pNext = nullptr;
      info.flags = 0u;
      info.image = dataVk->myImage;
      info.format = RenderCore_PlatformVk::ResolveFormat(someProperties.myFormat);
      info.viewType = locGetImageViewType(someProperties.myDimension);
      info.subresourceRange.baseArrayLayer = subresourceRange.myFirstArrayIndex;
      info.subresourceRange.layerCount = subresourceRange.myNumArrayIndices;
      info.subresourceRange.baseMipLevel = subresourceRange.myFirstMipLevel;
      info.subresourceRange.levelCount = subresourceRange.myNumMipLevels;
      info.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
      info.subresourceRange.aspectMask = 0u;
      if (formatInfo.myIsDepthStencil)
      {
        const bool viewHasDepth = subresourceRange.myFirstPlane == 0;
        if (viewHasDepth)
          info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;

        const bool viewHasStencil = subresourceRange.myFirstPlane + subresourceRange.myNumPlanes == 2u;
        if (viewHasStencil)
          info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
      }
      else
      {
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      }

      VkImageView imageView = nullptr;
      ASSERT_VK_RESULT(vkCreateImageView(RenderCore::GetPlatformVk()->myDevice, &info, nullptr, &imageView));

      return imageView;
    }
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  TextureViewVk::TextureViewVk(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties)
    : TextureView(aTexture, someProperties)
  {
    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(someProperties.myFormat);

    String name = aTexture->myName;

    GpuResourceViewDataVk nativeData;
    nativeData.myType = GpuResourceViewDataVk::Image;
    myType = GpuResourceViewType::NONE;
    if (someProperties.myIsRenderTarget)
    {
      if (formatInfo.myIsDepthStencil)
      {
        myType = GpuResourceViewType::DSV;
        name.append(" DSV");
      }
      else
      {
        myType = GpuResourceViewType::RTV;
        name.append(" RTV");
      }
    }
    else
    {
      if (someProperties.myIsShaderWritable)
      {
        myType = GpuResourceViewType::UAV;
        name.append(" UAV");
      }
      else
      {
        myType = GpuResourceViewType::SRV;
        name.append(" SRV");
      }
    }

    nativeData.myView.myImage = Priv_TextureViewVk::locCreateImageView(aTexture.get(), someProperties);
    ASSERT(nativeData.myView.myImage != nullptr && myType != GpuResourceViewType::NONE);
    myNativeData = nativeData;

    const TextureProperties& texProps = aTexture->GetProperties();
    const uint numTexMips = texProps.myNumMipLevels;
    const uint numTexArraySlices = texProps.GetArraySize();

    mySubresources->reserve(aTexture->myNumSubresources);

    const TextureSubRange& subresourceRange = someProperties.mySubresourceRange;

    if (myType != GpuResourceViewType::DSV)
    {
      ASSERT(subresourceRange.myFirstPlane == 0u && subresourceRange.myNumPlanes == 1u);

      for (uint iArray = subresourceRange.myFirstArrayIndex, e = subresourceRange.myFirstArrayIndex + subresourceRange.myNumArrayIndices; iArray < e; ++iArray)
        for (uint iMip = subresourceRange.myFirstMipLevel, eMip = subresourceRange.myFirstMipLevel + subresourceRange.myNumMipLevels; iMip < eMip; ++iMip)
          mySubresources[0].push_back(TextureVk::CalcSubresourceIndex(iMip, numTexMips, iArray, numTexArraySlices, 0u));

      myCoversAllSubresources = mySubresources[0].size() == aTexture->myNumSubresources;
    }
    else // DSV
    {
      ASSERT(formatInfo.myNumPlanes <= GpuResourceView::ourNumSupportedPlanes);
      for (int i = 0; i < (int)formatInfo.myNumPlanes; ++i)
      {
        for (uint iArray = subresourceRange.myFirstArrayIndex, e = subresourceRange.myFirstArrayIndex + subresourceRange.myNumArrayIndices; iArray < e; ++iArray)
          for (uint iMip = subresourceRange.myFirstMipLevel, eMip = subresourceRange.myFirstMipLevel + subresourceRange.myNumMipLevels; iMip < eMip; ++iMip)
            mySubresources[i].push_back(TextureVk::CalcSubresourceIndex(iMip, numTexMips, iArray, numTexArraySlices, i));
      }

      myCoversAllSubresources = mySubresources[0].size() == aTexture->myNumSubresourcesPerPlane;
    }
  }
//---------------------------------------------------------------------------//
  TextureViewVk::~TextureViewVk()
  {
    const GpuResourceViewDataVk& viewDataVk = myNativeData.To<GpuResourceViewDataVk>();
    vkDestroyImageView(RenderCore::GetPlatformVk()->myDevice, viewDataVk.myView.myImage, nullptr);
  }
//---------------------------------------------------------------------------//
}
