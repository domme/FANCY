#include "stdafx.h"
#include "TextureViewGroup.h"
#include "RenderCore.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  void TextureViewGroup::Create(const TextureViewGroupProperties& someProps, const char* aName)
  {
    ASSERT(!someProps.myIsShaderWritable || someProps.myTextureProperties.myIsShaderWritable);
    ASSERT(!someProps.myIsRenderTarget || someProps.myTextureProperties.myIsRenderTarget);

    myTexture = RenderCore::CreateTexture(someProps.myTextureProperties, aName);
    ASSERT(myTexture);

    if (someProps.myIsTexture)
    {
      TextureViewProperties viewProps;
      myReadTextureView = RenderCore::CreateTextureView(myTexture, viewProps);
      ASSERT(myReadTextureView);
    }

    if (someProps.myIsRenderTarget)
    {
      TextureViewProperties viewProps;
      viewProps.myIsRenderTarget = true;
      viewProps.myMipIndex = 0u;
      viewProps.myNumMipLevels = 1u;
      myRenderTargetView = RenderCore::CreateTextureView(myTexture, viewProps);
      ASSERT(myRenderTargetView);
    }

    if (someProps.myIsShaderWritable)
    {
      TextureViewProperties viewProps;
      viewProps.myIsShaderWritable = true;
      viewProps.myMipIndex = 0u;
      viewProps.myNumMipLevels = 1u;
      viewProps.myFormat = DataFormatInfo::GetNonSRGBformat(someProps.myTextureProperties.eFormat);
      myWriteTextureView = RenderCore::CreateTextureView(myTexture, viewProps);
      ASSERT(myWriteTextureView);
    }
  }
//---------------------------------------------------------------------------//
}


