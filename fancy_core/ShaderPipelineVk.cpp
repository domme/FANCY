#include "fancy_core_precompile.h"
#include "ShaderPipelineVk.h"
#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  void ShaderPipelineVk::CreateFromShaders()
  {
#if FANCY_HEAVY_DEBUG
    const bool hasComputeShader = myShaders[(uint)ShaderStage::COMPUTE] != nullptr;
    for (uint i = 0u; i < (uint)ShaderStage::NUM; ++i)
    {
      if (myShaders[i] == nullptr)
        continue;

      if (hasComputeShader)
      {
        ASSERT(i == (uint)ShaderStage::COMPUTE, "Can't mix a compute shader with other stages in the same pipeline");
      }
      else
      {
        ASSERT(i != (uint)ShaderStage::COMPUTE, "Can't mix a compute shader with other stages in the same pipeline");
      }
    }
#endif
  }
}

#endif
