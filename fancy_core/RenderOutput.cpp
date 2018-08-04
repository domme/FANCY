#include "RenderOutput.h"
#include "Window.h"
#include "RenderCore.h"
#include "TextureViewProperties.h"
#include "Texture.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  RenderOutput::RenderOutput(void* aNativeInstanceHandle)
    : myCurrBackbufferIndex(0u)
  {
    Fancy::WindowParameters params;
    params.myTitle = "Fancy";
    params.myWidth = 1280u;
    params.myHeight = 720u;

    HINSTANCE instanceHandle = static_cast<HINSTANCE>(aNativeInstanceHandle);

    myWindow = Window::Create(instanceHandle, params);
    myWindow->myOnResize.Connect(this, &RenderOutput::OnWindowResized);
  }
//---------------------------------------------------------------------------//
  RenderOutput::~RenderOutput()
  {
    myWindow->myOnResize.DetachObserver(this);
  }
//---------------------------------------------------------------------------//
  void RenderOutput::PrepareForFirstFrame()
  {
    uint width, height;
    GetWindowSizeSafe(width, height);

    CreateBackbuffer(width, height);
    CreateViews();
  }
//---------------------------------------------------------------------------//
  void RenderOutput::GetWindowSizeSafe(uint& aWidthOut, uint& aHeightOut)
  {
    aWidthOut = glm::max(8u, myWindow->GetWidth());
    aHeightOut = glm::max(8u, myWindow->GetHeight());
  }
//---------------------------------------------------------------------------//
  void RenderOutput::OnWindowResized(uint aWidth, uint aHeight)
  {
    GetWindowSizeSafe(aWidth, aHeight);
    ResizeBackbuffer(aWidth, aHeight);

    for (uint i = 0u; i < kBackbufferCount; i++)
    {
      myBackbufferTextures[i]->myProperties.myWidth = aWidth;
      myBackbufferTextures[i]->myProperties.myHeight = aHeight;
    }

    CreateViews();
  }
//---------------------------------------------------------------------------//
  void RenderOutput::CreateViews()
  {
    uint width, height;
    GetWindowSizeSafe(width, height);

    TextureProperties dsTexProps;
    dsTexProps.myDimension = GpuResourceDimension::TEXTURE_2D;
    dsTexProps.bIsDepthStencil = true;
    dsTexProps.eFormat = DataFormat::D_24UNORM_S_8UI;
    dsTexProps.myIsRenderTarget = false;
    dsTexProps.myIsShaderWritable = false;
    dsTexProps.myWidth = width;
    dsTexProps.myHeight = height;
    dsTexProps.myNumMipLevels = 1u;

    SharedPtr<Texture> dsTexture = RenderCore::CreateTexture(dsTexProps);
    ASSERT(dsTexture != nullptr);

    // DepthStencil DSVs
    {
      TextureViewProperties props;
      props.myDimension = GpuResourceDimension::TEXTURE_2D;
      props.myIsRenderTarget = true;
      props.myFormat = DataFormat::D_24UNORM_S_8UI;
      myDepthStencilDsv = RenderCore::CreateTextureView(dsTexture, props);
      ASSERT(myDepthStencilDsv != nullptr);

      props.myIsDepthReadOnly = true;
      props.myIsStencilReadOnly = true;
      myDepthStencilDsv_ReadOnly = RenderCore::CreateTextureView(dsTexture, props);
      ASSERT(myDepthStencilDsv_ReadOnly != nullptr);
    }

    // Depth SRV
    {
      TextureViewProperties props;
      props.myDimension = GpuResourceDimension::TEXTURE_2D;
      props.myFormat = DataFormat::D_24UNORM_S_8UI;
      myDepthSrv = RenderCore::CreateTextureView(dsTexture, props);
      ASSERT(myDepthSrv != nullptr);
    }

    for (uint i = 0u; i < kBackbufferCount; i++)
    {
      ASSERT(myBackbufferTextures[i] != nullptr && myBackbufferTextures[i]->GetProperties().myWidth == width && myBackbufferTextures[i]->GetProperties().myHeight == height);

      // Backbuffer RTV
      {
        TextureViewProperties props;
        props.myDimension = GpuResourceDimension::TEXTURE_2D;
        props.myFormat = DataFormat::RGBA_8;
        props.myIsRenderTarget = true;
        myBackbufferRtv[i] = RenderCore::CreateTextureView(myBackbufferTextures[i], props);
        ASSERT(myBackbufferRtv[i] != nullptr);
      }

      // Backbuffer SRV
      {
        TextureViewProperties props;
        props.myDimension = GpuResourceDimension::TEXTURE_2D;
        props.myFormat = DataFormat::RGBA_8;
        myBackbufferSrv[i] = RenderCore::CreateTextureView(myBackbufferTextures[i], props);
        ASSERT(myBackbufferSrv[i] != nullptr);
      }
    }
  }
//---------------------------------------------------------------------------//  
}
