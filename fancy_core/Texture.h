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
  public:
    Texture();
    virtual ~Texture();

    virtual void Create(const TextureParams& clDeclaration, const TextureSubData* someInitialDatas = nullptr, uint aNumInitialDatas = 0u) = 0;
    virtual void GetSubresourceLayout(const TextureSubLocation& aStartSubLocation, uint aNumSubDatas, DynamicArray<TextureSubLayout>& someLayoutsOut, DynamicArray<uint64>& someOffsetsOut, uint64& aTotalSizeOut) const = 0;
    virtual uint GetSubresourceIndex(const TextureSubLocation& aSubresourceLocation) const = 0;
    virtual TextureSubLocation GetSubresourceLocation(uint aSubresourceIndex) const = 0;

    const TextureParams& GetParameters() const { return myParameters; }
    const bool IsArray() const { return myParameters.myDimension == GpuResourceDimension::TEXTURE_1D_ARRAY || myParameters.myDimension == GpuResourceDimension::TEXTURE_2D_ARRAY || myParameters.myDimension == GpuResourceDimension::TEXTURE_CUBE_ARRAY; }
    const uint GetArraySize() const { return IsArray() ? myParameters.myDepthOrArraySize : 0u; }
    
  protected:
    TextureParams myParameters;
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
