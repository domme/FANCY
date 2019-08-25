#include "fancy_core_precompile.h"
#include "TextureVk.h"
#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"
#include "GpuResourceDataVk.h"

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
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  void TextureVk::GetSubresourceLayout(const TextureSubLocation& aStartSubLocation, uint aNumSubDatas, DynamicArray<TextureSubLayout>& someLayoutsOut, DynamicArray<uint64>& someOffsetsOut, uint64& aTotalSizeOut) const
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
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
  TextureViewVk::TextureViewVk(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties)
    : TextureView(aTexture, someProperties)
  {
  }
//---------------------------------------------------------------------------//
  TextureViewVk::~TextureViewVk()
  {
  }
//---------------------------------------------------------------------------//
  bool TextureViewVk::CreateSRV(const Texture* aTexture, const TextureViewProperties& someProperties, const DescriptorVk& aDescriptor)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
    return false;
  }
//---------------------------------------------------------------------------//
  bool TextureViewVk::CreateUAV(const Texture* aTexture, const TextureViewProperties& someProperties, const DescriptorVk& aDescriptor)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
    return false;
  }
//---------------------------------------------------------------------------//
  bool TextureViewVk::CreateRTV(const Texture* aTexture, const TextureViewProperties& someProperties, const DescriptorVk& aDescriptor)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
    return false;
  }
//---------------------------------------------------------------------------//
  bool TextureViewVk::CreateDSV(const Texture* aTexture, const TextureViewProperties& someProperties, const DescriptorVk& aDescriptor)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
    return false;
  }
//---------------------------------------------------------------------------//
}
