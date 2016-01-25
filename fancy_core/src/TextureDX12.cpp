#include "StdAfx.h"

#if defined (RENDERER_DX12)

#include "TextureDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  TextureDX12::TextureDX12()
  {
  }
//---------------------------------------------------------------------------//
  TextureDX12::~TextureDX12()
  {
  }
//---------------------------------------------------------------------------//
  bool TextureDX12::operator==(const TextureDesc& aDesc) const
  {
    return GetDescription() == aDesc;
  }
//---------------------------------------------------------------------------//
  TextureDesc TextureDX12::GetDescription() const
  {
    TextureDesc desc;

    desc.myIsExternalTexture = myParameters.myIsExternalTexture;
    desc.myInternalRefIndex = myParameters.myInternalRefIndex;
    desc.mySourcePath = myParameters.path;

    return desc;
  }
//---------------------------------------------------------------------------//
  void TextureDX12::SetFromDescription(const TextureDesc& aDesc)
  {
    // TODO: Read from binary cache

    myParameters.myIsExternalTexture = aDesc.myIsExternalTexture;
    myParameters.myInternalRefIndex = aDesc.myInternalRefIndex;
    myParameters.path = aDesc.mySourcePath;
  }
//---------------------------------------------------------------------------//
  void TextureDX12::create(const TextureCreationParams& clDeclaration, CreationMethod eCreationMethod)
  {

  }
//---------------------------------------------------------------------------//
  void TextureDX12::setPixelData(void* pData, uint uDataSizeBytes, glm::u32vec3 rectPosOffset, glm::u32vec3 rectDimensions)
  {

  }
//---------------------------------------------------------------------------//
  void* TextureDX12::lock(GpuResoruceLockOption option)
  {
    return nullptr;
  }
//---------------------------------------------------------------------------//
  void TextureDX12::unlock()
  {
  }
//---------------------------------------------------------------------------//
} } }

#endif
