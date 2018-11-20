#include "stdafx.h"
#include "TextureResource.h"
#include "RenderCore.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  void TextureResource::Update(const TextureResourceProperties& someProps, const char* aName)
  {
    const TextureProperties& texProps = someProps.myTextureProperties;
    ASSERT(!someProps.myIsShaderWritable || texProps.myIsShaderWritable);
    ASSERT(!someProps.myIsRenderTarget || texProps.myIsRenderTarget);

    bool needsCreate = myTexture == nullptr || 
      (someProps.myIsTexture == (myReadView == nullptr)) ||
      (someProps.myIsRenderTarget == (myRenderTargetView == nullptr)) ||
      (someProps.myIsShaderWritable == (myWriteView == nullptr));

    if (!needsCreate)
    {
      const TextureProperties& currTexProps = myTexture->GetProperties();
      needsCreate =
        texProps.myWidth != currTexProps.myWidth ||
        texProps.myHeight != currTexProps.myHeight ||
        texProps.GetDepthSize() != currTexProps.GetDepthSize() ||
        texProps.GetArraySize() != currTexProps.GetArraySize() ||
        texProps.eFormat != currTexProps.eFormat ||
        texProps.myDimension != currTexProps.myDimension ||
        texProps.myPreferTypedFormat != currTexProps.myPreferTypedFormat;
    }

    if (needsCreate)
    {
      myTexture = RenderCore::CreateTexture(someProps.myTextureProperties, aName);
      ASSERT(myTexture);

      if (someProps.myIsTexture)
      {
        TextureViewProperties viewProps;
        myReadView = RenderCore::CreateTextureView(myTexture, viewProps);
        ASSERT(myReadView);
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
        myWriteView = RenderCore::CreateTextureView(myTexture, viewProps);
        ASSERT(myWriteView);
      }
    }

    myTexture->SetName(aName);
  }
//---------------------------------------------------------------------------//
}