#pragma once


#include "VkPrerequisites.h"
#include "Texture.h"
#include "DynamicArray.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct GpuResourceDataVk;

  class TextureVk final : public Texture
  {
  public:
    TextureVk() = default;
    TextureVk(GpuResource&& aResource, const TextureProperties& someProperties, bool aIsSwapChainTexture);
    ~TextureVk() override;

    bool IsValid() const override;
    void SetName(const char* aName) override;

    void Create(const TextureProperties& someProperties, const char* aName = nullptr, const TextureSubData* someInitialDatas = nullptr, uint aNumInitialDatas = 0u) override;

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
  };
//---------------------------------------------------------------------------//
}
