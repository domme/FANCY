#pragma once

#include "Texture.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct GpuResourceDataDX12;
  struct DescriptorDX12;

  class TextureDX12 final : public Texture
  {
    friend class RenderOutputDX12;  // Remove after backbuffers are handled through the texture class

  public:
    TextureDX12();
    ~TextureDX12() override;

    static uint CalcSubresourceIndex(uint aMipIndex, uint aNumMips, uint anArrayIndex, uint aNumArraySlices, uint aPlaneIndex);
    static uint CalcNumSubresources(uint aNumMips, uint aNumArraySlices, uint aNumPlanes);

    bool IsValid() const override;
    void SetName(const char* aName) override;

    void Create(const TextureProperties& someProperties, const char* aName = nullptr, const TextureSubData* someInitialDatas = nullptr, uint aNumInitialDatas = 0u) override;
    void GetSubresourceLayout(const TextureSubLocation& aStartSubLocation, uint aNumSubDatas, DynamicArray<TextureSubLayout>& someLayoutsOut, DynamicArray<uint64>& someOffsetsOut, uint64& aTotalSizeOut) const override;
    uint GetSubresourceIndex(const TextureSubLocation& aSubresourceLocation) const override;
    uint GetNumSubresources() const override;
    uint GetNumSubresourcesPerPlane() const override;
    TextureSubLocation GetSubresourceLocation(uint aSubresourceIndex) const override;
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
