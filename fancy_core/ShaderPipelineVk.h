#pragma once
#include "ShaderPipeline.h"
#include "VkPrerequisites.h"

namespace Fancy
{
  class ShaderPipelineVk : public ShaderPipeline
  {
  public:
    ~ShaderPipelineVk() override;

    void UpdateResourceInterface() override;

    VkPipelineLayout myPipelineLayout;
  };
}



