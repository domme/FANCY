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
}
