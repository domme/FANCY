#pragma once
#include "ShaderPipeline.h"
#include "VkPrerequisites.h"
#include "PipelineLayoutVk.h"
#include "eastl/vector.h"

#if FANCY_ENABLE_VK

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