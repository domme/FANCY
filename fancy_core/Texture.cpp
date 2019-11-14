#include "fancy_core_precompile.h"
#include "Texture.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  Texture::Texture()
    : GpuResource(GpuResourceCategory::TEXTURE)
    , myIsSwapChainTexture(false)
  {
  }
//---------------------------------------------------------------------------//
  Texture::Texture(GpuResource&& aResource, const TextureProperties& someProperties, bool aIsSwapChainTexture)
    : GpuResource(std::move(aResource))
    , myProperties(someProperties)
    , myIsSwapChainTexture(aIsSwapChainTexture)
  {
  }
//---------------------------------------------------------------------------//
}
