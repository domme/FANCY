#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "Serializable.h"
#include "GpuResource.h"
#include "TextureDesc.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct DLLEXPORT TextureInfos {
    TextureInfos() : isSRGB(0), isLocked(0), isArrayTexture(0), isCubemap(0),
      cachesTextureData(0), numDimensions(0) {}

    uint isSRGB : 1;
    uint isLocked : 1;
    uint isArrayTexture : 1;
    uint isCubemap : 1;
    uint cachesTextureData : 1;
    uint numDimensions : 4;
  };
//---------------------------------------------------------------------------//
  class DLLEXPORT Texture : public GpuResource
  {
  public:
    SERIALIZABLE_RESOURCE(Texture)
    
    Texture();
    virtual ~Texture();

    bool operator==(const TextureDesc& aDesc) const;
    TextureDesc GetDescription() const;
    void SetFromDescription(const TextureDesc& aDesc);

    virtual void Create(const TextureParams& clDeclaration, const TextureUploadData* someInitialDatas = nullptr, uint32 aNumInitialDatas = 0u) = 0;

    const TextureInfos& GetStateInfos() const { return myState; }
    const TextureParams& GetParameters() const { return myParameters; }

  protected:
    TextureParams myParameters;
    TextureInfos myState;
  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering
