#include "fancy_core_precompile.h"

#if FANCY_ENABLE_VK

#include "TextureVk.h"

#include "Rendering/GlobalDescriptorAllocation.h"
#include "Rendering/RenderCore.h"

#include "RenderCore_PlatformVk.h"
#include "GpuResourceDataVk.h"
#include "GpuResourceViewDataVk.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  TextureVk::TextureVk(GpuResource&& aResource, const TextureProperties& someProperties, bool aIsSwapChainTexture)
    : Texture(std::move(aResource), someProperties, aIsSwapChainTexture)
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
    const GpuResourceDataVk* nativeData = GetData();
    return nativeData != nullptr && nativeData->myImage != nullptr;
  }
//---------------------------------------------------------------------------//
  void TextureVk::SetName(const char* aName)
  {
    Texture::SetName(aName);

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    const GpuResourceDataVk* const nativeData = GetData();
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
    ASSERT((aNumInitialDatas == 0) == (someInitialDatas == nullptr));

    Destroy();

    GpuResourceDataVk dataVk;
    dataVk.myType = GpuResourceType::TEXTURE;
    
    myProperties = someProperties;
    
    bool isArray = false;
    bool isCube = false;
    const VkImageType imageType = RenderCore_PlatformVk::ResolveImageResourceDimension(myProperties.myDimension, isArray, isCube);

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = nullptr;

    imageInfo.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    if (isCube)
      imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    if (isArray && imageType == VK_IMAGE_TYPE_2D)
      imageInfo.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;

    imageInfo.imageType = imageType;
    imageInfo.format = RenderCore_PlatformVk::ResolveFormat(DataFormatInfo::GetNonSRGBformat(myProperties.myFormat));
    imageInfo.extent.width = myProperties.myWidth;
    imageInfo.extent.height = myProperties.myHeight;
    imageInfo.extent.depth = glm::max(1u, myProperties.GetDepthSize());
    imageInfo.arrayLayers = glm::max(1u, myProperties.GetArraySize());
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    
    const uint minSide = (myProperties.myDimension == GpuResourceDimension::TEXTURE_3D) ? glm::min(myProperties.myWidth, myProperties.myHeight, myProperties.myDepthOrArraySize) : glm::min(myProperties.myWidth, myProperties.myHeight);
    const uint maxNumMipLevels = 1u + static_cast<uint>(glm::floor(glm::log2(minSide)));
    imageInfo.mipLevels = glm::max(1u, glm::min(myProperties.myNumMipLevels, maxNumMipLevels));
    myProperties.myNumMipLevels = imageInfo.mipLevels;

    VkAccessFlags readMask = VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_MEMORY_READ_BIT;
    VkAccessFlags writeMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT;

    uint supportedImageLayoutMask = 0u;
    supportedImageLayoutMask |= RenderCore_PlatformVk::ImageLayoutToFlag(VK_IMAGE_LAYOUT_GENERAL);
    supportedImageLayoutMask |= RenderCore_PlatformVk::ImageLayoutToFlag(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    supportedImageLayoutMask |= RenderCore_PlatformVk::ImageLayoutToFlag(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    supportedImageLayoutMask |= RenderCore_PlatformVk::ImageLayoutToFlag(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(myProperties.myFormat);
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    if (myProperties.myIsRenderTarget)
    {
      if (formatInfo.myIsDepthStencil)
      {
        readMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        writeMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        supportedImageLayoutMask |= RenderCore_PlatformVk::ImageLayoutToFlag(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        supportedImageLayoutMask |= RenderCore_PlatformVk::ImageLayoutToFlag(VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL);
        supportedImageLayoutMask |= RenderCore_PlatformVk::ImageLayoutToFlag(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL);
        supportedImageLayoutMask |= RenderCore_PlatformVk::ImageLayoutToFlag(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
        supportedImageLayoutMask |= RenderCore_PlatformVk::ImageLayoutToFlag(VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL);
        supportedImageLayoutMask |= RenderCore_PlatformVk::ImageLayoutToFlag(VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL);
        supportedImageLayoutMask |= RenderCore_PlatformVk::ImageLayoutToFlag(VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL);
      }
      else
      {
        readMask |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        writeMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        supportedImageLayoutMask |= RenderCore_PlatformVk::ImageLayoutToFlag(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      }  
    }
    if (myProperties.myIsShaderWritable)
    {
      imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;

      writeMask |= VK_ACCESS_SHADER_WRITE_BIT;
    }

    const RenderPlatformCaps& caps = RenderCore::GetPlatformCaps();
    const bool hasAsyncQueues = caps.myHasAsyncCompute || caps.myHasAsyncCopy;

    imageInfo.sharingMode = hasAsyncQueues ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    const uint queueFamilyIndices[] =
    {
      (uint)platformVk->GetQueueInfo(CommandListType::Graphics).myQueueFamilyIndex,
      (uint)platformVk->GetQueueInfo(CommandListType::Compute).myQueueFamilyIndex,
    };
    imageInfo.pQueueFamilyIndices = queueFamilyIndices;
    imageInfo.queueFamilyIndexCount = caps.myHasAsyncCompute ? ARRAY_LENGTH(queueFamilyIndices) : 1;
    
    mySubresources = SubresourceRange(0u, myProperties.myNumMipLevels, 0u, myProperties.GetArraySize(), 0, formatInfo.myNumPlanes);

    GpuResourceHazardDataVk& hazardData = dataVk.myHazardData;
    hazardData = GpuResourceHazardDataVk();

    hazardData.myReadAccessMask = readMask;
    hazardData.myWriteAccessMask = writeMask;
    hazardData.myHasExclusiveQueueAccess = imageInfo.sharingMode == VK_SHARING_MODE_EXCLUSIVE;
    hazardData.mySupportedImageLayoutMask = supportedImageLayoutMask;

    const bool hasInitData = someInitialDatas != nullptr && aNumInitialDatas > 0u;
    const VkImageLayout initialImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    GpuSubresourceHazardDataVk subHazardData;
    subHazardData.myContext = CommandListType::Graphics;
    subHazardData.myAccessMask = 0u;
    subHazardData.myImageLayout = initialImageLayout; // Initial layout must be either UNDEFINED or PREINITIALIZED
    hazardData.mySubresources.resize(mySubresources.GetNumSubresources(), subHazardData);
    
    imageInfo.initialLayout = initialImageLayout;

    VkDevice device = platformVk->myDevice;
    ASSERT_VK_RESULT(vkCreateImage(device, &imageInfo, nullptr, &dataVk.myImage));

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, dataVk.myImage, &memRequirements);

    const uint memoryTypeIndex = platformVk->FindMemoryTypeIndex(memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    VkMemoryAllocateInfo memAllocInfo;
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.pNext = nullptr;
    memAllocInfo.allocationSize = memRequirements.size;
    memAllocInfo.memoryTypeIndex = memoryTypeIndex;
    ASSERT_VK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &dataVk.myMemory));
    ASSERT_VK_RESULT(vkBindImageMemory(device, dataVk.myImage, dataVk.myMemory, 0));

    myNativeData = dataVk;

    SetName(aName != nullptr ? aName : "Texture_Unnamed");

    if (hasInitData)
      InitTextureData(someInitialDatas, aNumInitialDatas);
  }
//---------------------------------------------------------------------------//
  GpuResourceDataVk* TextureVk::GetData()
  {
    return !myNativeData.has_value() ? nullptr : eastl::any_cast<GpuResourceDataVk>(&myNativeData);
  }
//---------------------------------------------------------------------------//
  const GpuResourceDataVk* TextureVk::GetData() const
  {
    return !myNativeData.has_value() ? nullptr : eastl::any_cast<GpuResourceDataVk>(&myNativeData);
  }
//---------------------------------------------------------------------------//
  void TextureVk::Destroy()
  {
    if (IsValid())
    {
      const GpuResourceDataVk* const dataVk = GetData();
      RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

      if (!myIsSwapChainTexture)  // Memory is managed by the swapchain and shouldn't be released here
        vkDestroyImage(platformVk->myDevice, dataVk->myImage, nullptr);
    }

    myNativeData.reset();
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
      const SubresourceRange& subresourceRange = someProperties.mySubresourceRange;

      VkImageViewUsageCreateInfo usageInfo = {};
      usageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO;
      usageInfo.pNext = nullptr;
      
      usageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
      if (someProperties.myIsShaderWritable)
      {
        usageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
      }
      else if (someProperties.myIsRenderTarget)
      {
        if (formatInfo.myIsDepthStencil)
        {
          usageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
        else
        {
          usageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
      }
      else
      {
        usageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
      }
      
      VkImageViewCreateInfo info;
      info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      info.pNext = &usageInfo;
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
  TextureViewVk::TextureViewVk(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties, const char* aName)
    : TextureView(aTexture, someProperties, aName)
  {
    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(someProperties.myFormat);

    eastl::string name = myName.empty() ? aTexture->myName : myName;

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

    if (myType == GpuResourceViewType::SRV || myType == GpuResourceViewType::UAV)
    {
      VkDescriptorImageInfo imageInfo = {};
      imageInfo.imageLayout = myType == GpuResourceViewType::SRV ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;
      imageInfo.imageView = nativeData.myView.myImage;
      imageInfo.sampler = nullptr;

      nativeData.myGlobalDescriptor = RenderCore::GetPlatformVk()->AllocateAndWriteGlobalResourceDescriptor(GetGlobalResourceType(someProperties), imageInfo, name.data());
      myGlobalDescriptorIndex = nativeData.myGlobalDescriptor.myIndex;
    }

    myNativeData = nativeData;
    
    const TextureProperties& texProps = aTexture->GetProperties();
    const uint numTexMips = texProps.myNumMipLevels;
    const uint numTexArraySlices = texProps.GetArraySize();
    
    const SubresourceRange& subresourceRange = someProperties.mySubresourceRange;
    mySubresourceRange = subresourceRange;

    myCoversAllSubresources = subresourceRange.myNumMipLevels == texProps.myNumMipLevels
      && subresourceRange.myNumArrayIndices == texProps.myDepthOrArraySize
      && subresourceRange.myNumPlanes == DataFormatInfo::GetFormatInfo(texProps.myFormat).myNumPlanes;
  }
//---------------------------------------------------------------------------//
  TextureViewVk::~TextureViewVk()
  {
    const GpuResourceViewDataVk& viewDataVk = eastl::any_cast<const GpuResourceViewDataVk&>(myNativeData);

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    platformVk->FreeGlobalResourceDescriptor(viewDataVk.myGlobalDescriptor);
    vkDestroyImageView(platformVk->myDevice, viewDataVk.myView.myImage, nullptr);
  }
//---------------------------------------------------------------------------//
}

#endif