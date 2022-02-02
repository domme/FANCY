#include "fancy_core_precompile.h"

#if FANCY_ENABLE_VK

#include "Rendering/RenderCore.h"

#include "TextureSamplerVk.h"
#include "RenderCore_PlatformVk.h"

namespace Fancy
{
  namespace Priv_TextureSamplerVk
  {
//---------------------------------------------------------------------------//
    VkSamplerAddressMode locResolveAddressMode(SamplerAddressMode aMode)
    {
      switch(aMode)
      {
      case SamplerAddressMode::CLAMP_EDGE: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
      case SamplerAddressMode::MIRROR_CLAMP_EDGE: return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
      case SamplerAddressMode::CLAMP_BORDER: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
      case SamplerAddressMode::REPEAT: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
      case SamplerAddressMode::MIRROR_REPEAT: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
      default: ASSERT(false); return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
      }
    }
 //---------------------------------------------------------------------------//
    VkBorderColor locResolveBorderColor(SamplerBorderColor aColor)
    {
      switch(aColor)
      {
      case SamplerBorderColor::FLOAT_TRANSPARENT_BLACK: return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
      case SamplerBorderColor::INT_TRANSPARENT_BLACK: return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
      case SamplerBorderColor::FLOAT_OPAQUE_BLACK: return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
      case SamplerBorderColor::INT_OPAQUE_BLACK: return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
      case SamplerBorderColor::FLOAT_OPAQUE_WHITE: return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
      case SamplerBorderColor::INT_OPAQUE_WHITE: return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
      default: ASSERT(false); return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
      }
    }
  //---------------------------------------------------------------------------//
    VkCompareOp locResolveCompFunc(CompFunc aFunc)
    {
      switch(aFunc)
      {
      case CompFunc::NEVER: return VK_COMPARE_OP_NEVER;
      case CompFunc::LESS: return VK_COMPARE_OP_LESS;
      case CompFunc::EQUAL: return VK_COMPARE_OP_EQUAL;
      case CompFunc::LEQUAL: return VK_COMPARE_OP_LESS_OR_EQUAL;
      case CompFunc::GREATER: return VK_COMPARE_OP_GREATER;
      case CompFunc::NOTEQUAL: return VK_COMPARE_OP_NOT_EQUAL;
      case CompFunc::GEQUAL: return VK_COMPARE_OP_GREATER_OR_EQUAL;
      case CompFunc::ALWAYS: return VK_COMPARE_OP_ALWAYS;
      default: ASSERT(false); return VK_COMPARE_OP_ALWAYS;
      }
    }
//---------------------------------------------------------------------------//
  }
//---------------------------------------------------------------------------//
  TextureSamplerVk::TextureSamplerVk(const TextureSamplerProperties& someProperties)
    : TextureSampler(someProperties)
    , mySampler(nullptr)
  {
    VkSamplerCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0u;
    info.addressModeU = Priv_TextureSamplerVk::locResolveAddressMode(myProperties.myAddressModeX);
    info.addressModeV = Priv_TextureSamplerVk::locResolveAddressMode(myProperties.myAddressModeY);
    info.addressModeW = Priv_TextureSamplerVk::locResolveAddressMode(myProperties.myAddressModeZ);
    info.borderColor = Priv_TextureSamplerVk::locResolveBorderColor(myProperties.myBorderColor);
    info.compareEnable = true; // Note: This doesn't exist on DX12. Let's hope it will default back to the shader-state if this doesn't match.
    info.compareOp = Priv_TextureSamplerVk::locResolveCompFunc(myProperties.myComparisonFunc);

    info.anisotropyEnable = false;
    info.maxAnisotropy = 0.0f;

    switch (myProperties.myMinFiltering)
    {
    case SamplerFilterMode::NEAREST:
      info.minFilter = VK_FILTER_NEAREST;
      info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
      break;
    case SamplerFilterMode::BILINEAR:
      info.minFilter = VK_FILTER_LINEAR;
      info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
      break;
    case SamplerFilterMode::TRILINEAR:
      info.minFilter = VK_FILTER_LINEAR;
      info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
      break;
    case SamplerFilterMode::ANISOTROPIC:
      info.minFilter = VK_FILTER_LINEAR;
      info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
      ASSERT(myProperties.myMaxAnisotropy > 0.0f);
      info.anisotropyEnable = true;
      info.maxAnisotropy = (float) glm::min(RenderCore::GetPlatformCaps().myMaxTextureAnisotropy, myProperties.myMaxAnisotropy);
      break;
    default: ASSERT(false);
    }

    switch (myProperties.myMagFiltering)
    {
    case SamplerFilterMode::NEAREST: 
      info.magFilter = VK_FILTER_NEAREST;
      break;
    case SamplerFilterMode::BILINEAR:
    case SamplerFilterMode::TRILINEAR:
    case SamplerFilterMode::ANISOTROPIC:
      info.magFilter = VK_FILTER_LINEAR;
      break;
    default: ASSERT(false);
    }

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    ASSERT_VK_RESULT(vkCreateSampler(platformVk->myDevice, &info, nullptr, &mySampler));

    VkDescriptorImageInfo descriptorInfo;
    descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    descriptorInfo.imageView = nullptr;
    descriptorInfo.sampler = mySampler;

    myDescriptorAllocation = platformVk->AllocateAndWriteGlobalResourceDescriptor(GLOBAL_RESOURCE_SAMPLER, descriptorInfo, "Sampler");
    myGlobalDescriptorIndex = myDescriptorAllocation.myIndex;
  }
//---------------------------------------------------------------------------//
  TextureSamplerVk::~TextureSamplerVk()
  {
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    vkDestroySampler(platformVk->myDevice, mySampler, nullptr);

    platformVk->FreeGlobalResourceDescriptor(myDescriptorAllocation);
  }
//---------------------------------------------------------------------------//
}

#endif