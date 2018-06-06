#include "RenderOutput.h"
#include "Window.h"
#include "RenderCore.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  RenderOutput::RenderOutput(void* aNativeInstanceHandle)
    : myCurrBackbufferIndex(0u)
    , myDefaultDepthStencil(nullptr)
  {
    memset(myBackbuffers, 0u, sizeof(myBackbuffers));

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
  void RenderOutput::CreateBackbufferResources()
  {
    TextureParams dsTexParams;
    dsTexParams.bIsDepthStencil = true;
    dsTexParams.eFormat = DataFormat::DS_24_8;
    dsTexParams.myIsExternalTexture = false;
    dsTexParams.myIsRenderTarget = false;
    dsTexParams.myIsShaderWritable = false;
    dsTexParams.u16Width = myWindow->GetWidth();
    dsTexParams.u16Height = myWindow->GetHeight();
    dsTexParams.myNumMipLevels = 1u;

    myDefaultDepthStencil = RenderCore::CreateTexture(dsTexParams);
    ASSERT(myDefaultDepthStencil != nullptr);

    for (uint i = 0u; i < kBackbufferCount; i++)
    {
      TextureParams backbufferParams;
      backbufferParams.myIsRenderTarget = true;
      backbufferParams.eFormat = DataFormat::RGBA_8;
      backbufferParams.u16Width = myWindow->GetWidth();
      backbufferParams.u16Height = myWindow->GetHeight();

      myBackbuffers[i] = RenderCore::CreateTexture(backbufferParams);
    }
  }
//---------------------------------------------------------------------------//  
}