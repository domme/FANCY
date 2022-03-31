#include "fancy_core_precompile.h"
#include "RenderOutput.h"

#include "Common/Window.h"
#include "Common/TimeManager.h"
#include "RenderCore.h"
#include "TextureProperties.h"
#include "Texture.h"
#include "CommandQueue.h"
#include "CommandList.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  RenderOutput::RenderOutput(void* aNativeInstanceHandle, const WindowParameters& someWindowParams)
    : myCurrBackbufferIndex(0u)
  {
    myWindow = Window::Create(reinterpret_cast<HINSTANCE>(aNativeInstanceHandle), someWindowParams);
    myWindow->myOnResize.Connect(this, &RenderOutput::OnWindowResized);
  }
//---------------------------------------------------------------------------//
  RenderOutput::~RenderOutput()
  {
    myWindow->myOnResize.DetachObserver(this);
  }
//---------------------------------------------------------------------------//
  void RenderOutput::BeginFrame()
  {
    if (Time::ourFrameIdx == 0u)
      PrepareForFirstFrame();

    OnBeginFrame();
  }
//---------------------------------------------------------------------------//
  void RenderOutput::EndFrame()
  {
    Present();
  }
//---------------------------------------------------------------------------//
  void RenderOutput::PrepareForFirstFrame()
  {
    uint width, height;
    GetWindowSizeSafe(width, height);

    CreateBackbufferResources(width, height);
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
    RenderCore::GetCommandQueue(CommandListType::Graphics)->WaitForIdle();

    DestroyViews();
    DestroyBackbufferResources();

    GetWindowSizeSafe(aWidth, aHeight);
    ResizeSwapChain(aWidth, aHeight);

    CreateBackbufferResources(aWidth, aHeight);
    CreateViews();
  }
//---------------------------------------------------------------------------//
  void RenderOutput::DestroyViews()
  {
    for (uint i = 0u; i < kBackbufferCount; ++i)
    {
      myBackbufferSrv[i].reset();
      myBackbufferRtv[i].reset();
    }
  }
//---------------------------------------------------------------------------//
  void RenderOutput::CreateViews()
  {
    uint width, height;
    GetWindowSizeSafe(width, height);

    for (uint i = 0u; i < kBackbufferCount; i++)
    {
      ASSERT(myBackbufferTextures[i] != nullptr && myBackbufferTextures[i]->GetProperties().myWidth == width && myBackbufferTextures[i]->GetProperties().myHeight == height);

      // Backbuffer RTV
      {
        TextureViewProperties props;
        props.myDimension = GpuResourceDimension::TEXTURE_2D;
        props.myFormat = myBackbufferTextures[i]->GetProperties().myFormat;
        props.myIsRenderTarget = true;
        myBackbufferRtv[i] = RenderCore::CreateTextureView(myBackbufferTextures[i], props);
        ASSERT(myBackbufferRtv[i] != nullptr);
      }

      // Backbuffer SRV
      {
        TextureViewProperties props;
        props.myDimension = GpuResourceDimension::TEXTURE_2D;
        props.myFormat = myBackbufferTextures[i]->GetProperties().myFormat;
        myBackbufferSrv[i] = RenderCore::CreateTextureView(myBackbufferTextures[i], props);
        ASSERT(myBackbufferSrv[i] != nullptr);
      }
    }
  }
//---------------------------------------------------------------------------//  
}
