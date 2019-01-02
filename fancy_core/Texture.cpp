#include "fancy_core_precompile.h"
#include "Texture.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  Texture::Texture()
    : GpuResource(GpuResourceCategory::TEXTURE)
  {
  }
//---------------------------------------------------------------------------//
  void Texture::Create(GpuResource&& aResource, const TextureProperties& someProperties)
  {
    Destroy();

    myProperties = someProperties;
    
    GpuResource& resource = *static_cast<GpuResource*>(this);
    resource = std::move(aResource);
  }
//---------------------------------------------------------------------------//
}
