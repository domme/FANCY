#pragma once
#include "ShaderPipeline.h"
#include "DX12Prerequisites.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
  class ShaderPipelineDX12 : public ShaderPipeline
  {
  public:
    void CreateFromShaders() override;
  };
}

#endif