#pragma once
#include "RenderOutput.h"
#include "VkPrerequisites.h"

namespace Fancy
{
  class RenderOutputVk : public RenderOutput
  {
  public:
    RenderOutputVk(void* aNativeInstanceHandle, const WindowParameters& someWindowParams);
    ~RenderOutputVk();

  protected:
    void CreateSwapChain();

    void CreateBackbufferResources(uint aWidth, uint aHeight) override;
    void ResizeBackbuffer(uint aWidth, uint aHeight) override;
    void DestroyBackbufferResources() override;
    void OnBeginFrame() override;
    void Present() override;

    VkFormat myBackbufferFormat = VK_FORMAT_UNDEFINED;
    VkColorSpaceKHR myBackbufferColorSpace = VK_COLOR_SPACE_MAX_ENUM_KHR;
    VkPresentModeKHR myPresentMode = VK_PRESENT_MODE_MAX_ENUM_KHR;

    VkSurfaceKHR mySurface = nullptr;
    VkSwapchainKHR mySwapChain = nullptr;
    VkFence myBackbufferReadyFence = nullptr;
    uint myNumBackbuffers = 0u;
    DynamicArray<bool> myBackbuffersUsed;
  };
}
