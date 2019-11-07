#include "fancy_core_precompile.h"
#include "Texture.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  Texture::Texture()
    : GpuResource(GpuResourceCategory::TEXTURE)
  {
  }
//---------------------------------------------------------------------------//
  Texture::Texture(GpuResource&& aResource, const TextureProperties& someProperties)
    : GpuResource(std::move(aResource))
    , myProperties(someProperties)
  {
  }
//---------------------------------------------------------------------------//
}
