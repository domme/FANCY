#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "GpuResource.h"
#include "TextureDesc.h"
#include "MathUtil.h"
#include <unordered_set>
#include <unordered_map>
#include "GpuResourceView.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class Texture : public GpuResource
  {
  public:
    Texture();
    virtual ~Texture();

    bool operator==(const TextureDesc& aDesc) const;
    TextureDesc GetDescription() const;
    void SetFromDescription(const TextureDesc& aDesc);

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
  struct TextureViewProperties
  {
    TextureViewProperties()
      : myDimension(GpuResourceDimension::UNKONWN)
      , myFormat(DataFormat::NONE)
      , myIsShaderWritable(false)
      , myIsRenderTarget(false)
      , myIsDepthStencilReadOnly(false)
      , myNumMipLevels(1u)
      , myPlaneIndex(0u)
      , myArraySize(0u)
      , myFirstArrayIndex(0u)
      , myMinLodClamp(0.0f)
      , myMipIndex(0u)
      , myFirstZindex(0u)
      , myZSize(0u)
    { }

    GpuResourceDimension myDimension;
    DataFormat myFormat;
    bool myIsShaderWritable;
    bool myIsRenderTarget;
    bool myIsDepthStencilReadOnly;
    uint myNumMipLevels;
    uint myPlaneIndex;
    uint myArraySize;        // Interpreted as NumCubes in case of cube arrays
    uint myFirstArrayIndex;  // Interpreted as First 2D Array face in case of cube arrays
    float myMinLodClamp;
    uint myMipIndex; // Only rendertargets
    uint myFirstZindex;  // Only rendertargets
    uint myZSize; // Only rendertargets
  };
//---------------------------------------------------------------------------//
  struct TextureView : public GpuResourceView
  {
    TextureView(SharedPtr<Texture> aTexture, UniquePtr<GpuResourceViewData> aData, const TextureViewProperties& someProperties)
      : GpuResourceView(std::static_pointer_cast<GpuResource>(aTexture), std::move(aData))
      , myProperties(someProperties)
    { }

    const TextureViewProperties& GetProperties() const { return myProperties; }
    Texture* GetTexture() const { return static_cast<Texture*>(myResource.get()); }

  protected:
    TextureViewProperties myProperties;
  };
//---------------------------------------------------------------------------//
}
