#pragma once
#include "RenderOutput.h"

namespace Fancy
{
  class RenderOutputVk : public RenderOutput
  {
  public:
    RenderOutputVk(void* aNativeInstanceHandle, const WindowParameters& someWindowParams);
    ~RenderOutputVk();

  protected:
    void CreateBackbufferResources(uint aWidth, uint aHeight) override;
    void ResizeBackbuffer(uint aWidth, uint aHeight) override;
    void DestroyBackbufferResources() override;
    void OnBeginFrame() override;
    void Present() override;
  };
}


