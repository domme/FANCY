#pragma once

#include "Texture.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct TextureViewGroupProperties
  {
    TextureProperties myTextureProperties;
    bool myIsTexture;
    bool myIsShaderWritable;
    bool myIsRenderTarget;
  };
//---------------------------------------------------------------------------//
  struct TextureViewGroup
  {
    void Create(const TextureViewGroupProperties& someProps, const char* aName = nullptr);

    SharedPtr<Texture> myTexture;
    SharedPtr<TextureView> myReadTextureView;
    SharedPtr<TextureView> myWriteTextureView;
    SharedPtr<TextureView> myRenderTargetView;
  };
//---------------------------------------------------------------------------//
}