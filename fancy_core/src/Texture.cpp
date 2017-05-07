#include "Texture.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  Texture::Texture()
  {
  }
//---------------------------------------------------------------------------//
  Texture::~Texture()
  {
  }
//---------------------------------------------------------------------------//
  bool Texture::operator==(const TextureDesc& aDesc) const
  {
    return GetDescription() == aDesc;
  }
//---------------------------------------------------------------------------//
  TextureDesc Texture::GetDescription() const
  {
    TextureDesc desc;
    desc.myIsExternalTexture = myParameters.myIsExternalTexture;
    desc.myInternalRefIndex = myParameters.myInternalRefIndex;
    desc.mySourcePath = myParameters.path;
    return desc;
  }
//---------------------------------------------------------------------------//
  void Texture::SetFromDescription(const TextureDesc& aDesc)
  {
    // TODO: Read from binary cache

    myParameters.myIsExternalTexture = aDesc.myIsExternalTexture;
    myParameters.myInternalRefIndex = aDesc.myInternalRefIndex;
    myParameters.path = aDesc.mySourcePath;
  }
//---------------------------------------------------------------------------//
} }