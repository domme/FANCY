#pragma once

#include "Common/FancyCoreDefines.h"
#include "GpuResource.h"
#include "GpuResourceView.h"
#include "TextureProperties.h"
#include "TextureData.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class Texture : public GpuResource
  {
    friend class RenderOutput;  // Needed to modify implicit SwapChain-texture properties

  public:
    Texture();
    Texture(GpuResource&& aResource, const TextureProperties& someProperties, bool aIsSwapChainTexture);
    virtual ~Texture() = default;

    static void ComputeRowPitchSizeAndBlockHeight(DataFormat aFormat, uint aWidth, uint aHeight, uint64& rowPitchSize, uint& aHeightBlocksOrPixels);

    virtual void Create(const TextureProperties& someProperties, const char* aName = nullptr, const TextureSubData* someInitialDatas = nullptr, uint aNumInitialDatas = 0u) = 0;

    const TextureProperties& GetProperties() const { return myProperties; }
    bool IsSwapChainTexture() const { return myIsSwapChainTexture; }
    
  protected:
    virtual void Destroy() = 0;

    void InitTextureData(const TextureSubData* someInitialDatas, uint aNumInitialDatas);

    TextureProperties myProperties;
    bool myIsSwapChainTexture;
  };
//---------------------------------------------------------------------------//
  class TextureView : public GpuResourceView
  {
  public:
    static GlobalResourceType GetGlobalResourceType(const TextureViewProperties& someViewProps);

    TextureView(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties)
      : GpuResourceView(eastl::static_pointer_cast<GpuResource>(aTexture))
      , myProperties(someProperties)
    { }

    const TextureViewProperties& GetProperties() const { return myProperties; }
    Texture* GetTexture() const { return static_cast<Texture*>(myResource.get()); }
    SharedPtr<Texture> GetTexturePtr() const { return eastl::static_pointer_cast<Texture>(myResource); }

  protected:
    TextureViewProperties myProperties;
  };
//---------------------------------------------------------------------------//
}
