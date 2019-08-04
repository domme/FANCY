#pragma once
#include "RenderOutput.h"
#include "VkPrerequisites.h"

namespace Fancy
{
  class RenderOutputVk : public RenderOutput
  {
  public:
    RenderOutputVk(HINSTANCE aNativeInstanceHandle, const WindowParameters& someWindowParams);
    ~RenderOutputVk();

  protected:
    void CreateBackbufferResources(uint aWidth, uint aHeight) override;
    void ResizeBackbuffer(uint aWidth, uint aHeight) override;
    void DestroyBackbufferResources() override;
    void OnBeginFrame() override;
    void Present() override;

    VkSurfaceKHR mySurface;
  };
}


