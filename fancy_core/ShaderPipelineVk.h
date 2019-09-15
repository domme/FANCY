#pragma once
#include "ShaderPipeline.h"

namespace Fancy
{
  class ShaderPipelineVk : public ShaderPipeline
  {
  public:
    void UpdateResourceInterface() override;
  };
}



