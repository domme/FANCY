#include "fancy_core_precompile.h"
#include "ShaderVk.h"
#include "VkPrerequisites.h"
#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"
#include "ShaderCompiler.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  namespace Priv_ShaderVk
  {
  //---------------------------------------------------------------------------//
    VkShaderStageFlagBits locResolveShaderStage(ShaderStage aShaderStage)
    {
      switch (aShaderStage)
      {
        case ShaderStage::VERTEX: return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderStage::FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderStage::GEOMETRY: return VK_SHADER_STAGE_GEOMETRY_BIT;
        case ShaderStage::TESS_HULL: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case ShaderStage::TESS_DOMAIN: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case ShaderStage::COMPUTE: return VK_SHADER_STAGE_COMPUTE_BIT;
        default: ASSERT(false, "Missing implementation"); return VK_SHADER_STAGE_VERTEX_BIT;
      }
    }
  //---------------------------------------------------------------------------//
  }
//---------------------------------------------------------------------------//
  ShaderVk::~ShaderVk()
  {
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    vkDestroyShaderModule(platformVk->myDevice, myCompiledData.myModule, nullptr);
  }
//---------------------------------------------------------------------------//
  void ShaderVk::SetFromCompilerOutput(const ShaderCompilerResult& aCompilerOutput)
  {
    Shader::SetFromCompilerOutput(aCompilerOutput);
    
    if (myCompiledData.myModule != nullptr)
    {
      RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
      vkDestroyShaderModule(platformVk->myDevice, myCompiledData.myModule, nullptr);
    }

    const ShaderCompiledDataVk& data = eastl::any_cast<const ShaderCompiledDataVk&>(aCompilerOutput.myNativeData);
    myCompiledData = data;

    myShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    myShaderStageCreateInfo.pNext = nullptr;
    myShaderStageCreateInfo.flags = 0;
    myShaderStageCreateInfo.pName = myDesc.myMainFunction.c_str();
    myShaderStageCreateInfo.pSpecializationInfo = nullptr;
    myShaderStageCreateInfo.module = myCompiledData.myModule;
    myShaderStageCreateInfo.stage = Priv_ShaderVk::locResolveShaderStage((ShaderStage) myDesc.myShaderStage);
  }
//---------------------------------------------------------------------------//
}

#endif