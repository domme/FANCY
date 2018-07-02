#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "GpuResource.h"
#include "TextureDesc.h"
#include "MathUtil.h"
#include <unordered_set>
#include <unordered_map>

namespace Fancy {
  //---------------------------------------------------------------------------//
  struct TextureViewProperties
  {
    TextureViewProperties()
      : myDimension(TextureDimension::UNKONWN)
      , myFormat(DataFormat::NONE)
      , myIsShaderWritable(false)
      , myIsRenderTarget(false)
      , myNumMipLevels(1u)
      , myPlaneIndex(0u)
      , myArraySize(0u)
      , myFirstArrayIndex(0u)
      , myMinLodClamp(0.0f)
      , myMipIndex(0u)
      , myFirstZindex(0u)
      , myZSize(0u)
    { }

    TextureDimension myDimension;
    DataFormat myFormat;
    bool myIsShaderWritable;
    bool myIsRenderTarget;
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
}
//---------------------------------------------------------------------------//
namespace std
{
  template <> struct std::hash<Fancy::TextureViewProperties>
  {
    std::size_t operator() (const Fancy::TextureViewProperties& someProperties) const
    {
      return static_cast<size_t>(Fancy::MathUtil::ByteHash(someProperties));
    }
  };
}
//---------------------------------------------------------------------------//

namespace Fancy {
//---------------------------------------------------------------------------//
  struct TextureView
  {
    TextureViewProperties myProperties;
    Descriptor myDescriptor;
  };
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

    TextureView* 
    const TextureParams& GetParameters() const { return myParameters; }
    const bool IsArray() const { return myParameters.myDimension == TextureDimension::TEXTURE_1D_ARRAY || myParameters.myDimension == TextureDimension::TEXTURE_2D_ARRAY || myParameters.myDimension == TextureDimension::TEXTURE_CUBE_ARRAY; }
    const uint GetArraySize() const { return IsArray() ? myParameters.myDepthOrArraySize : 0u; }
    
  protected:
    TextureParams myParameters;
    std::unordered_map<TextureViewProperties, UniquePtr<TextureView>> myViews;
  };
//---------------------------------------------------------------------------//
}
