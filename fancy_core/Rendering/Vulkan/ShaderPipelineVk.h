#pragma once

#if FANCY_ENABLE_VK

#include "Rendering/ShaderPipeline.h"

namespace Fancy
{
  struct ShaderResourceInfoVk;

  class ShaderPipelineVk : public ShaderPipeline
  {
  public:
    void CreateFromShaders() override;
  };
}

#endif