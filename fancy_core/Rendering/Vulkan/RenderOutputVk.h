#pragma once

#if FANCY_ENABLE_VK

#include "Rendering/RenderOutput.h"

#include "VkPrerequisites.h"

namespace Fancy
{
  class RenderOutputVk : public RenderOutput
  {
  public:
    RenderOutputVk(void* aNativeInstanceHandle, const WindowParameters& someWindowParams);
    ~RenderOutputVk();

  protected:
    void CreateSwapChain(uint aWidth, uint aHeight);
    void DestroySwapChain();

    void CreateBackbufferResources(uint aWidth, uint aHeight) override;
    void ResizeSwapChain(uint aWidth, uint aHeight) override;
    void DestroyBackbufferResources() override;
    void OnBeginFrame() override;
    void Present() override;

    VkFormat myBackbufferFormat = VK_FORMAT_UNDEFINED;
    VkColorSpaceKHR myBackbufferColorSpace = VK_COLOR_SPACE_MAX_ENUM_KHR;
    VkPresentModeKHR myPresentMode = VK_PRESENT_MODE_MAX_ENUM_KHR;
    VkSharingMode mySharingMode = VK_SHARING_MODE_MAX_ENUM;

    VkSurfaceKHR mySurface = nullptr;
    VkSwapchainKHR mySwapChain = nullptr;
    VkFence myBackbufferReadyFence = nullptr;
    uint myNumBackbuffers = 0u;
  };
}

#endif