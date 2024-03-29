#include "fancy_core_precompile.h"

#if FANCY_ENABLE_VK

#include "Rendering/RenderCore.h"

#include "ShaderPipelineVk.h"
#include "RenderCore_PlatformVk.h"

namespace Fancy
{
  void ShaderPipelineVk::CreateFromShaders()
  {
#if FANCY_HEAVY_DEBUG
    const bool hasComputeShader = myShaders[(uint)ShaderStage::SHADERSTAGE_COMPUTE] != nullptr;
    for (uint i = 0u; i < (uint)ShaderStage::SHADERSTAGE_NUM; ++i)
    {
      if (myShaders[i] == nullptr)
        continue;

      if (hasComputeShader)
      {
        ASSERT(i == (uint)ShaderStage::SHADERSTAGE_COMPUTE, "Can't mix a compute shader with other stages in the same pipeline");
      }
      else
      {
        ASSERT(i != (uint)ShaderStage::SHADERSTAGE_COMPUTE, "Can't mix a compute shader with other stages in the same pipeline");
      }
    }
#endif
  }
}

#endif
