#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "GpuResource.h"
#include "TextureViewProperties.h"
#include "GpuResourceView.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class Texture : public GpuResource
  {
    friend class RenderOutput;  // Needed to modify implicit SwapChain-texture properties

  public:
    Texture();
    virtual ~Texture() = default;

    void Create(GpuResource&& aResource, const TextureProperties& someProperties);
    virtual void Create(const TextureProperties& someProperties, const char* aName = nullptr, const TextureSubData* someInitialDatas = nullptr, uint aNumInitialDatas = 0u) = 0;
    virtual void GetSubresourceLayout(const TextureSubLocation& aStartSubLocation, uint aNumSubDatas, DynamicArray<TextureSubLayout>& someLayoutsOut, DynamicArray<uint64>& someOffsetsOut, uint64& aTotalSizeOut) const = 0;
    virtual uint GetSubresourceIndex(const TextureSubLocation& aSubresourceLocation) const = 0;
    virtual uint GetNumSubresourcesPerPlane() const = 0;
    virtual TextureSubLocation GetSubresourceLocation(uint aSubresourceIndex) const = 0;

    const TextureProperties& GetProperties() const { return myProperties; }
    
  protected:
    virtual void Destroy() = 0;

    TextureProperties myProperties;
  };
//---------------------------------------------------------------------------//
  class TextureView : public GpuResourceView
  {
  public:
    TextureView(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties)
      : GpuResourceView(std::static_pointer_cast<GpuResource>(aTexture))
      , myProperties(someProperties)
    { }

    const TextureViewProperties& GetProperties() const { return myProperties; }
    Texture* GetTexture() const { return static_cast<Texture*>(myResource.get()); }

  protected:
    TextureViewProperties myProperties;
  };
//---------------------------------------------------------------------------//
}
