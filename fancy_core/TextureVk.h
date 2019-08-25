#pragma once

#include "Texture.h"
#include "DynamicArray.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct GpuResourceDataVk;
  class DescriptorVk;

  class TextureVk final : public Texture
  {
    friend class RenderOutputVk;  // Remove after backbuffers are handled through the texture class

  public:
    TextureVk() = default;
    TextureVk(GpuResource&& aResource, const TextureProperties& someProperties);
    ~TextureVk() override;

    bool IsValid() const override;
    void SetName(const char* aName) override;

    void Create(const TextureProperties& someProperties, const char* aName = nullptr, const TextureSubData* someInitialDatas = nullptr, uint aNumInitialDatas = 0u) override;
    void GetSubresourceLayout(const TextureSubLocation& aStartSubLocation, uint aNumSubDatas, DynamicArray<TextureSubLayout>& someLayoutsOut, DynamicArray<uint64>& someOffsetsOut, uint64& aTotalSizeOut) const override;

    GpuResourceDataVk* GetData() const;
  protected:
    void Destroy() override;
  };
//---------------------------------------------------------------------------//
  class TextureViewVk final : public TextureView
  {
  public:
    TextureViewVk(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties);
    ~TextureViewVk() override;

  private:
    static bool CreateSRV(const Texture* aTexture, const TextureViewProperties& someProperties, const DescriptorVk& aDescriptor);
    static bool CreateUAV(const Texture* aTexture, const TextureViewProperties& someProperties, const DescriptorVk& aDescriptor);
    static bool CreateRTV(const Texture* aTexture, const TextureViewProperties& someProperties, const DescriptorVk& aDescriptor);
    static bool CreateDSV(const Texture* aTexture, const TextureViewProperties& someProperties, const DescriptorVk& aDescriptor);
  };
//---------------------------------------------------------------------------//
}
