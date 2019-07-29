#include "fancy_core_precompile.h"
#include "GraphicsResources.h"

#include "RenderCore.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  void GpuBufferResource::Update(const GpuBufferResourceProperties& someProps, const char* aName)
  {
    const GpuBufferProperties& newProps = someProps.myBufferProperties;
    ASSERT(!someProps.myIsShaderWritable || newProps.myIsShaderWritable);
    
    bool needsCreate = myBuffer == nullptr ||
      (someProps.myIsShaderResource == (myReadView == nullptr)) ||
      (someProps.myIsShaderWritable == (myWriteView == nullptr));

    if (!needsCreate)
    {
      const GpuBufferProperties& currProps = myBuffer->GetProperties();
      needsCreate =
        newProps.myElementSizeBytes != currProps.myElementSizeBytes ||
        newProps.myNumElements != currProps.myNumElements ||
        newProps.myCpuAccess != currProps.myCpuAccess ||
        newProps.myUsage != currProps.myUsage;

      if (myReadView != nullptr || myWriteView != nullptr)
      {
        const GpuBufferViewProperties& currViewProps = myReadView != nullptr ? myReadView->GetProperties() : myWriteView->GetProperties();

        needsCreate |= currViewProps.myIsRaw != someProps.myIsRaw ||
          currViewProps.myFormat != someProps.myFormat ||
          currViewProps.myIsStructured != someProps.myIsStructured;
      }
    }

    if (needsCreate)
    {
      myBuffer = RenderCore::CreateBuffer(newProps, aName);
      ASSERT(myBuffer);

      GpuBufferViewProperties viewProps;
      viewProps.myFormat = someProps.myFormat;
      viewProps.myIsRaw = someProps.myIsRaw;
      viewProps.myIsStructured = someProps.myIsStructured;

      if (someProps.myIsShaderResource)
      {
        myReadView = RenderCore::CreateBufferView(myBuffer, viewProps);
        ASSERT(myReadView);
      }

      if (someProps.myIsShaderWritable)
      {
        viewProps.myIsShaderWritable = true;
        myWriteView = RenderCore::CreateBufferView(myBuffer, viewProps);
        ASSERT(myWriteView);
      }
    }

    myBuffer->SetName(aName);
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  void TextureResource::Update(const TextureResourceProperties& someProps, const char* aName)
  {
    const TextureProperties& texProps = someProps.myTextureProperties;

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
      TextureProperties createProps = texProps;
      createProps.path.clear();
      createProps.myIsShaderWritable = someProps.myIsShaderWritable;
      createProps.myIsRenderTarget = someProps.myIsRenderTarget;

      myTexture = RenderCore::CreateTexture(createProps, aName);
      ASSERT(myTexture);

      if (someProps.myIsTexture)
      {
        const TextureViewProperties viewProps;
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