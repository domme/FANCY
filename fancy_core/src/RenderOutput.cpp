#include "RenderOutput.h"
#include "RenderWindow.h"
#include "RenderCore.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  RenderOutput::RenderOutput(void* aNativeInstanceHandle)
    : myCurrBackbufferIndex(0u)
  {
    Fancy::WindowParameters params;
    params.myTitle = "Fancy Engine";
    params.myWidth = 800u;
    params.myHeight = 600u;

    HINSTANCE instanceHandle = static_cast<HINSTANCE>(aNativeInstanceHandle);

    myWindow = RenderWindow::Create(instanceHandle, params);
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
    dsTexParams.u8NumMipLevels = 1u;

    myDefaultDepthStencil = RenderCore::CreateTexture(dsTexParams);
    ASSERT(myDefaultDepthStencil != nullptr);

    for (uint32 i = 0u; i < kBackbufferCount; i++)
    {
      TextureParams backbufferParams;
      backbufferParams.myIsRenderTarget = true;
      backbufferParams.eFormat = DataFormat::RGBA_8;
      backbufferParams.u16Width = myWindow->GetWidth();
      backbufferParams.u16Height = myWindow->GetHeight();
      backbufferParams.u16Depth = 1u;

      myBackbuffers[i] = RenderCore::CreateTexture(backbufferParams);
    }
  }
//---------------------------------------------------------------------------//  
} }
