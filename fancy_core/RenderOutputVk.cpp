#include "fancy_core_precompile.h"
#include "RenderOutputVk.h"

namespace Fancy
{
  RenderOutputVk::RenderOutputVk(void* aNativeInstanceHandle, const WindowParameters& someWindowParams)
    : RenderOutput(aNativeInstanceHandle, someWindowParams)
  {
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
