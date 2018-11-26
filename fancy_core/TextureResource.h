#pragma once

#include "Texture.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct TextureResourceProperties
  {
    TextureProperties myTextureProperties;
    bool myIsTexture;
    bool myIsShaderWritable;
    bool myIsRenderTarget;
  };
//---------------------------------------------------------------------------//
  struct TextureResource
  {
    void Update(const TextureResourceProperties& someProps, const char* aName = nullptr);

    SharedPtr<Texture> myTexture;
    SharedPtr<TextureView> myReadView;
    SharedPtr<TextureView> myWriteView;
    SharedPtr<TextureView> myRenderTargetView;
  };
//---------------------------------------------------------------------------//
}