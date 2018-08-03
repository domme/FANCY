#include "RenderOutput.h"
#include "Window.h"
#include "RenderCore.h"
#include "TextureViewProperties.h"

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
  void RenderOutput::OnWindowResized(uint aWidth, uint aHeight)
  {
    const uint width = glm::max(8u, myWindow->GetWidth());
    const uint height = glm::max(8u, myWindow->GetHeight());

    CreateBackbufferResources();
  }
//---------------------------------------------------------------------------//
  void RenderOutput::CreateViews()
  {
    

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
      TextureProperties backbufferProps;
      backbufferProps.myDimension = GpuResourceDimension::TEXTURE_2D;
      backbufferProps.myIsRenderTarget = true;
      backbufferProps.eFormat = DataFormat::RGBA_8;
      backbufferProps.myWidth = width;
      backbufferProps.myHeight = height;

      SharedPtr<Texture> backbuffer = RenderCore::CreateTexture(backbufferProps);

      // Backbuffer RTV
      {
        TextureViewProperties props;
        props.myDimension = GpuResourceDimension::TEXTURE_2D;
        props.myFormat = DataFormat::RGBA_8;
        props.myIsRenderTarget = true;
        myBackbufferRtv[i] = RenderCore::CreateTextureView(backbuffer, props);
        ASSERT(myBackbufferRtv[i] != nullptr);
      }

      // Backbuffer SRV
      {
        TextureViewProperties props;
        props.myDimension = GpuResourceDimension::TEXTURE_2D;
        props.myFormat = DataFormat::RGBA_8;
        myBackbufferSrv[i] = RenderCore::CreateTextureView(backbuffer, props);
        ASSERT(myBackbufferSrv[i] != nullptr);
      }
    }
  }
//---------------------------------------------------------------------------//  
}
