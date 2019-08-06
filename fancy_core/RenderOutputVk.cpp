#include "fancy_core_precompile.h"
#include "RenderOutputVk.h"
#include "Window.h"
#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"

namespace Fancy
{
  class RenderCore_PlatformVk;

  RenderOutputVk::RenderOutputVk(void* aNativeInstanceHandle, const WindowParameters& someWindowParams)
    : RenderOutput(aNativeInstanceHandle, someWindowParams)
  {
    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = myWindow->GetWindowHandle();
    createInfo.hinstance = reinterpret_cast<HINSTANCE>(aNativeInstanceHandle);

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    ASSERT_VK_RESULT(vkCreateWin32SurfaceKHR(platformVk->myInstance, &createInfo, nullptr, &mySurface));
  }

  RenderOutputVk::~RenderOutputVk()
  {
  }

  void RenderOutputVk::CreateBackbufferResources(uint aWidth, uint aHeight)
  {
  }

  void RenderOutputVk::ResizeBackbuffer(uint aWidth, uint aHeight)
  {
  }

  void RenderOutputVk::DestroyBackbufferResources()
  {
  }

  void RenderOutputVk::OnBeginFrame()
  {
  }

  void RenderOutputVk::Present()
  {
  }
}
