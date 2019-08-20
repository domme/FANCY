#include "fancy_core_precompile.h"
#include "TextureVk.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  TextureVk::~TextureVk()
  {
  }
//---------------------------------------------------------------------------//
  uint TextureVk::CalcSubresourceIndex(uint aMipIndex, uint aNumMips, uint anArrayIndex, uint aNumArraySlices, uint aPlaneIndex)
  {
  }
//---------------------------------------------------------------------------//
  uint TextureVk::CalcNumSubresources(uint aNumMips, uint aNumArraySlices, uint aNumPlanes)
  {
  }
//---------------------------------------------------------------------------//
  bool TextureVk::IsValid() const
  {
  }
//---------------------------------------------------------------------------//
  void TextureVk::SetName(const char* aName)
  {
  }
//---------------------------------------------------------------------------//
  void TextureVk::Create(const TextureProperties& someProperties, const char* aName, const TextureSubData* someInitialDatas, uint aNumInitialDatas)
  {
  }
//---------------------------------------------------------------------------//
  void TextureVk::GetSubresourceLayout(const TextureSubLocation& aStartSubLocation, uint aNumSubDatas, DynamicArray<TextureSubLayout>& someLayoutsOut, DynamicArray<uint64>& someOffsetsOut, uint64& aTotalSizeOut) const
  {
  }
//---------------------------------------------------------------------------//
  uint TextureVk::GetSubresourceIndex(const TextureSubLocation& aSubresourceLocation) const
  {
  }
//---------------------------------------------------------------------------//
  TextureSubLocation TextureVk::GetSubresourceLocation(uint aSubresourceIndex) const
  {
  }
//---------------------------------------------------------------------------//
  GpuResourceDataVk* TextureVk::GetData() const
  {
    return myNativeData.IsEmpty() ? nullptr : myNativeData.To<GpuResourceDataVk*>();
  }
//---------------------------------------------------------------------------//
  void TextureVk::Destroy()
  {
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
  }
//---------------------------------------------------------------------------//
  bool TextureViewVk::CreateUAV(const Texture* aTexture, const TextureViewProperties& someProperties, const DescriptorVk& aDescriptor)
  {
  }
//---------------------------------------------------------------------------//
  bool TextureViewVk::CreateRTV(const Texture* aTexture, const TextureViewProperties& someProperties, const DescriptorVk& aDescriptor)
  {
  }
//---------------------------------------------------------------------------//
  bool TextureViewVk::CreateDSV(const Texture* aTexture, const TextureViewProperties& someProperties, const DescriptorVk& aDescriptor)
  {
  }
//---------------------------------------------------------------------------//
}
