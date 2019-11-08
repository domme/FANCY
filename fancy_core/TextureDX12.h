#pragma once

#include "Texture.h"
#include "DynamicArray.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct GpuResourceDataDX12;
  class DescriptorDX12;

  class TextureDX12 final : public Texture
  {
    friend class RenderOutputDX12;  // Remove after backbuffers are handled through the texture class

  public:
    TextureDX12() = default;
    TextureDX12(GpuResource&& aResource, const TextureProperties& someProperties);
    ~TextureDX12() override;

    bool IsValid() const override;
    void SetName(const char* aName) override;

    void Create(const TextureProperties& someProperties, const char* aName = nullptr, const TextureSubData* someInitialDatas = nullptr, uint aNumInitialDatas = 0u) override;
    void GetSubresourceLayout(const SubresourceRange& aSubresourceRange, DynamicArray<TextureSubLayout>& someLayoutsOut, DynamicArray<uint64>& someOffsetsOut, uint64& aTotalSizeOut) const;

    GpuResourceDataDX12* GetData() const;
  protected:
    void Destroy() override;
  };
//---------------------------------------------------------------------------//
  class TextureViewDX12 final : public TextureView
  {
  public:
    TextureViewDX12(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties);
    ~TextureViewDX12() override;

  private:
    static bool CreateSRV(const Texture* aTexture, const TextureViewProperties& someProperties, const DescriptorDX12& aDescriptor);
    static bool CreateUAV(const Texture* aTexture, const TextureViewProperties& someProperties, const DescriptorDX12& aDescriptor);
    static bool CreateRTV(const Texture* aTexture, const TextureViewProperties& someProperties, const DescriptorDX12& aDescriptor);
    static bool CreateDSV(const Texture* aTexture, const TextureViewProperties& someProperties, const DescriptorDX12& aDescriptor);
  };
}
