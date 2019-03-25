#include "fancy_core_precompile.h"
#include "RenderOutput.h"

#include "Window.h"
#include "RenderCore.h"
#include "TextureProperties.h"
#include "Texture.h"
#include "CommandQueue.h"
#include "CommandContext.h"
#include "TimeManager.h"
#include "Profiler.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  RenderOutput::RenderOutput(void* aNativeInstanceHandle, const WindowParameters& someWindowParams)
    : myCurrBackbufferIndex(0u)
  {
    HINSTANCE instanceHandle = static_cast<HINSTANCE>(aNativeInstanceHandle);

    myWindow = Window::Create(instanceHandle, someWindowParams);
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
    Texture* currBackbuffer = myBackbufferRtv[myCurrBackbufferIndex]->GetTexture();

    CommandQueue* graphicsQueue = RenderCore::GetCommandQueue(CommandListType::Graphics);
    CommandContext* context = RenderCore::AllocateContext(CommandListType::Graphics);

    context->TransitionResource(currBackbuffer, GpuResourceTransition::TO_PRESENT);
    graphicsQueue->ExecuteContext(context);
    RenderCore::FreeContext(context);

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
    ResizeBackbuffer(aWidth, aHeight);

    CreateBackbufferResources(aWidth, aHeight);
    CreateViews();
  }
//---------------------------------------------------------------------------//
  void RenderOutput::DestroyViews()
  {
    myDepthSrv.reset();
    myDepthStencilDsv.reset();
    myDepthStencilDsv_ReadOnly.reset();
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

    TextureProperties dsTexProps;
    dsTexProps.myDimension = GpuResourceDimension::TEXTURE_2D;
    dsTexProps.bIsDepthStencil = true;
    dsTexProps.eFormat = DataFormat::D_24UNORM_S_8UI;
    dsTexProps.myIsRenderTarget = false;
    dsTexProps.myIsShaderWritable = false;
    dsTexProps.myWidth = width;
    dsTexProps.myHeight = height;
    dsTexProps.myNumMipLevels = 1u;

    SharedPtr<Texture> dsTexture = RenderCore::CreateTexture(dsTexProps, "Backbuffer DepthStencil Texture");
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
