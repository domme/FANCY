#include "fancy_core_precompile.h"
#include "Texture.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  Texture::Texture()
    : GpuResource(GpuResourceCategory::TEXTURE)
    , myIsPresentable(false)
  {
  }
//---------------------------------------------------------------------------//
  Texture::Texture(GpuResource&& aResource, const TextureProperties& someProperties, bool aIsPresentable)
    : GpuResource(std::move(aResource))
    , myProperties(someProperties)
    , myIsPresentable(aIsPresentable)
  {
  }
//---------------------------------------------------------------------------//
}
